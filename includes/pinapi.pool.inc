<?php 

/**
 * @file
 * Provides a generalized pool API for Drupal.
 */

/**
 * Lock a pool.
 */
function pinapi_pool_lock($pool_id) {
  db_query("UPDATE {pinapi_pool} SET locked = 1 WHERE pool_id = %d", $pool_id);
}

/**
 * Unlock a pool.
 */
function pinapi_pool_unlock($pool_id) {
  db_query("UPDATE {pinapi_pool} SET locked = 0 WHERE pool_id = %d", $pool_id);
}

/**
 * Determine whether a pool is locked.
 *
 * @param $pool_id
 *   A pool id.
 * @return
 *   Boolean
 */
function pinapi_pool_is_locked($pool_id) {
  return (boolean) db_result(db_query("SELECT locked FROM {pinapi_pool} WHERE pool_id = %d", $pool_id));
}


/**
 * Return an array of available prizes.
 *
 * @param $pool_id
 *   A pool_id.
 * @param $account
 *   A user object.
 * @return
 *   An array of prizes.
 */
function pinapi_pool_prizes($pool_id, $account = NULL, $reset = FALSE) {
  static $pool_prizes;

  if ( !isset($pool_prizes[$pool_id]) || $reset ) {
    $prizes = pinapi_prizes_select(array('pool_id' => $pool_id, 'active' => TRUE));

    // Remove any prizes that have a quantity of 0.
    foreach ( $prizes as $key => &$prize ) {
      _pinapi_invoke('prize_availability', $prize, $pool_id, $account);
      
      // Set a neutral bias on prize.
      if ( !isset($prizes[$key]['bias']) ) {
        $prizes[$key]['bias'] = 1;
      }
      
      // Removed prizes that haven't started yet.
      if ( !empty($prize['starts']) && $prize['starts'] != 0 && time() < $prize['starts'] ) {
        unset($prizes[$key]);
      }
      
      // Remove expired prizes.
      if ( !empty($prize['expires']) && $prize['expires'] != 0 && time() > $prize['expires'] ) {
        unset($prizes[$key]);
      }
      
      // Remove depleted prizes.
      if ( $prize['quantity'] == 0 ) {
        unset($prizes[$key]);
      }
    }

    // Let modules modify the prizes that are available to win.
    drupal_alter('pinapi_pool_prizes', $prizes, $pool_id, $account);

    $pool_prizes[$pool_id] = $prizes;
  }

  return $pool_prizes[$pool_id];
}

/**
 * Save a collection of pools to the database.
 */
function pinapi_pools_set(&$pools) {
  if ( !empty($pools['pool_id']) || !empty($pools['content_id']) ) {
    $pools = array($pools);
  }
  
  foreach ( $pools as $key => $pool ) {
    _pinapi_pools_prep($pool);
    _pinapi_invoke(isset($pool['pool_id']) ? 'pool_update' : 'pool_insert', $pool);
    drupal_write_record('pinapi_pool', $pool, isset($pool['pool_id']) ? 'pool_id' : array());
    $pools[$key] = $pool;
  }
  
  return $pools;
}

/**
 * Delete pools from the database.
 */
function pinapi_pools_delete($pools = array()) {
  if ( !empty($pools) ) {
    if ( !empty($pools['pool_id']) ) {
      $pools = array($pools);
    }
    
    $pool_ids = array();
    foreach ( $pools as $pool ) {
      _pinapi_invoke('pool_delete', $pool);
      $pool_ids[] = $pool['pool_id'];
    }
    
    db_query("DELETE FROM {pinapi_pool} WHERE pool_id IN (" . db_placeholders($pool_ids) . ")", $pool_ids);
  }
}

/**
 * Select individual pools from the database.
 */
function pinapi_pools_select($criteria = array(), $limit = 0) {
  $pools = array();
  $result = _pinapi_pools_select('pool', $criteria, $limit);
  while ( $pool = db_fetch_array($result) ) {
    _pinapi_invoke('pool_load', $pool);
    $pools[] = $pool;
  }
  
  return $pools;
}

/**
 * Select a single pool from the database.
 */
function pinapi_pools_select_single($criteria = array()) {
  if ( $pools = pinapi_pools_select($criteria) ) {
    return $pools[0];
  }
}

/**
 * Populate the value of any unset pool properties.
 */
function _pinapi_pools_prep(&$pool) {
  if ( !empty($pool['pool_id']) && empty($pool['prepped']) ) {
    $pool += array(
      'content_type' => 'node',
      'content_id' => 0,
      'locked' => 0,
      'status' => 1,
      'prepped' => TRUE,
    );
  }
}

/**
 * Internal helper function constructs SELECT queries.
 */
function _pinapi_pools_select($table = 'pool', $criteria = array(), $limit = 0) {
  $details = _pinapi_pools_query('pool', $criteria);
  $query = "SELECT * FROM {pinapi_" . $table . "} pool WHERE 1 = 1" . $details['query'];
  return $limit ? db_query_range($query, $details['args'], 0, $limit) : db_query($query, $details['args']);
}

/**
 * Internal helper function constructs DELETE queries.
 */
function _pinapi_pools_delete($table = 'pool', $criteria = array(), $limit = 0) {
  $details = _pinapi_pools_query('pool', $criteria, '');
  $query = "DELETE FROM {pinapi_" . $table . "} pool WHERE 1 = 1" . $details['query'];
  db_query($query, $details['args']);
}

/**
 * Internal helper function constructs WHERE clauses.
 */
function _pinapi_pools_query($table = 'pool', $criteria = array(), $alias = 'pool.') {
  $criteria += array(
    'pool_id' => NULL,
    'content_id' => NULL,
    'content_type' => NULL,
    'locked' => NULL,
    'status' => NULL,
    'active' => FALSE,
  );
  
  $query = '';
  $args = array();
  if ( !empty($criteria['pool_id']) ) {
    _pinapi_pools_query_builder($alias . 'pool_id', $criteria['pool_id'], $query, $args);
  }
  elseif ( !empty($criteria['active']) ) {
    _pinapi_pools_query_builder($alias . 'active', $criteria['active'], $query, $args);
    _pinapi_pools_query_builder($alias . 'pool_id', $criteria['pool_id'], $query, $args);
  }
  else {
    _pinapi_pools_query_builder($alias . 'content_id', $criteria['content_id'], $query, $args);
    _pinapi_pools_query_builder($alias . 'content_type', $criteria['content_type'], $query, $args, TRUE);
    _pinapi_pools_query_builder($alias . 'pool_id', $criteria['pool_id'], $query, $args);
    _pinapi_pools_query_builder($alias . 'locked', $criteria['locked'], $query, $args);
    _pinapi_pools_query_builder($alias . 'status', $criteria['status'], $query, $args);
  }
  
  return array('query' => $query, 'args' => $args);
}

/**
 * Internal helper function constructs individual elements of WHERE clauses.
 */
function _pinapi_pools_query_builder($name, $value, &$query, &$args, $col_is_string = FALSE) {
  if (!isset($value)) {
    // Do nothing
  }
  elseif ( $name == 'pool.active' && $value ) {
    $query .= " AND pool.locked = 1 AND pool.status = 1";
  }
  else {
    if (is_array($value)) {
      if ($col_is_string) {
        $query .= " AND $name IN (" . db_placeholders($value, 'varchar') . ")";
        $args = array_merge($args, $value);
      }
      else {
        $query .= " AND $name IN (" . db_placeholders($value, 'int') . ")";
        $args = array_merge($args, $value);
      }
    }
    else {
      if ($col_is_string) {
        $query .= " AND $name = '%s'";
        $args[] = $value;
      }
      else {
        $query .= " AND $name = %d";
        $args[] = $value;
      }
    }
  }
}
