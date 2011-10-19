<?php 

/**
 * @file
 * Provides a generalized prize API for Drupal.
 */

/**
 * Return a prize name.
 */
function pinapi_prize_name($prize) {
  $function = '_pinapi_prize_name_' . $prize['content_type'];
  if ( function_exists($function) ) {
    return $function($prize);
  }
}

/**
 * Return a prize associated object.
 */
function pinapi_prize_object($prize) {
  $function = '_pinapi_prize_object_' . $prize['content_type'];
  if ( function_exists($function) ) {
    return $function($prize);
  }
}

/**
 * Return a prize associated object.
 */
function pinapi_prize_operations($prize) {
  $function = '_pinapi_prize_operations_' . $prize['content_type'];
  if ( function_exists($function) ) {
    return $function($prize);
  }
}

/**
 * Return a prize link.
 */
function pinapi_prize_link($prize) {
  $function = '_pinapi_prize_link_' . $prize['content_type'];
  if ( function_exists($function) ) {
    return $function($prize);
  }
}

/**
 * Award a prize from a pool of prizes.
 *
 * @param $pool_id
 *   A pool id.
 * @param $account
 *   A user object.
 */
function pinapi_prize_award($pool_id, $account = NULL) {
  $prize = pinapi_prize_random($pool_id, $account);
  
  if ( $prize && $prize['quantity'] > 0 ) {
    $prize['decrease'] = TRUE;
    
    // Allow other modules to alter prize before it is awarded.
    _pinapi_invoke('prize_pre_award', $prize, $account);
    
    if ( $prize['decrease'] ) {
      db_query("UPDATE {pinapi_prize} SET quantity = quantity - 1 WHERE pid = %d", $prize['pid']);
    }
    
    // Allow other modules to alter prize after it is awarded.
    _pinapi_invoke('prize_award', $prize, $account);
    
    return $prize;
  }
  
  return FALSE;
}

/**
 * Choose a random prize from an array of prizes based on quantities.
 *
 * @param $pool_id
 *   A pool_id
 * @param $account
 *   A user account
 */
function pinapi_prize_random($pool_id, $account) {
  $prizes = pinapi_pool_prizes($pool_id, $account, TRUE);
  
  // Calculate a random number based on available prize quantities.
  $rand = rand(0, _pinapi_prize_pool_sum($prizes) - 1);
  
  // Sum up the totals and look for an appropriate match.
  $total = 0;
  foreach ( $prizes as $prize ) {
    $total += $prize['quantity'] * $prize['bias'];
    if ( $rand < $total ) {
      _pinapi_invoke('prize_random', $prize, $account);
      return $prize;
    }
  }
  
  return FALSE;
}

/**
 * Save a collection of prizes to the database.
 */
function pinapi_prizes_set(&$prizes, $override = FALSE) {
  if ( !empty($prizes['pid']) || !empty($prizes['content_id']) ) {
    $prizes = array($prizes);
  }
  
  foreach ( $prizes as $key => $prize ) {
    if ( !isset($prize['locked']) || !$prize['locked'] || $override ) {
      _pinapi_prizes_prep($prize);
      _pinapi_invoke(isset($prize['pid']) ? 'pool_update' : 'pool_insert', $prize);
      drupal_write_record('pinapi_prize', $prize, isset($prize['pid']) ? 'pid' : array());
      $prizes[$key] = $prize;
    }
  }
  
  return $prizes;
}

/**
 * Delete prizes from the database.
 */
function pinapi_prizes_delete($prizes = array()) {
  if ( !empty($prizes) ) {
    if ( !empty($prizes['pid']) ) {
      $prizes = array($prizes);
    }
    
    $pids = array();
    foreach ( $prizes as $prize ) {
      if ( !$prize['locked'] ) {
        _pinapi_invoke('prize_delete', $prize);
        $pids[] = $prize['pid'];
      }
    }
    
    db_query("DELETE FROM {pinapi_prize} WHERE pid IN (" . db_placeholders($pids) . ")", $pids);
  }
}

/**
 * Filter prizes from array.
 */
function pinapi_prizes_filter(&$prizes, $pid, $cond) {
  if ( $cond && isset($prizes[$pid]) ) {
    unset($prizes[$pid]);
  }
}

/**
 * Return an array of redeemed prizes for an account.
 */
function pinapi_prizes_redeemed($pool_id, $account) {
  $prizes = array();
  $result = db_query("SELECT * FROM {pinapi_code_redeemed} WHERE pool_id = %d AND uid = %d", $pool_id, $account->uid);
  while ( $redeemed = db_fetch_array($result) ) {
    $prizes[$redeemed['pid']] = $redeemed;
  }
  
  return $prizes;
}

/**
 * Determine if prize has already been awarded to account.
 */
function pinapi_prize_already_redeemed($pid, $pool_id, $account) {
  return (bool) db_result(db_query("SELECT COUNT(*) FROM {pinapi_code_redeemed} WHERE pid = %d AND pool_id = %d AND uid = %d", $pid, $pool_id, $account->uid));
}

/**
 * Select individual prizes from the database.
 */
function pinapi_prizes_select($criteria = array(), $limit = 0) {
  $prizes = array();
  $result = _pinapi_prizes_select('prize', $criteria, $limit);
  while ( $prize = db_fetch_array($result) ) {
    $prize['name'] = pinapi_prize_name($prize);
    _pinapi_invoke('prize_load', $prize);
    $prizes[$prize['pid']] = $prize;
  }
  
  return $prizes;
}

/**
 * Select a single prize from the database.
 */
function pinapi_prizes_select_single($criteria = array()) {
  if ( $prizes = pinapi_prizes_select($criteria) ) {
    return array_shift($prizes);
  }
}

/**
 * Populate the value of any unset prize properties.
 */
function _pinapi_prizes_prep(&$prize) {
  if ( !empty($prize['pid']) && empty($prize['prepped']) ) {
    $prize += array(
      'machine_name' => '',
      'content_type' => 'node',
      'quantity' => 0,
      'starts' => 0,
      'expires' => 0,
      'status' => 1,
      'prepped' => TRUE,
    );
  }
}

/**
 * Internal helper function constructs SELECT queries.
 */
function _pinapi_prizes_select($table = 'prize', $criteria = array(), $limit = 0) {
  $details = _pinapi_prizes_query('prize', $criteria);
  $query = "SELECT prize.*, pool.locked FROM {pinapi_" . $table . "} prize INNER JOIN {pinapi_pool} pool ON prize.pool_id = pool.pool_id WHERE 1 = 1" . $details['query'];
  return $limit ? db_query_range($query, $details['args'], 0, $limit) : db_query($query, $details['args']);
}

/**
 * Internal helper function constructs DELETE queries.
 */
function _pinapi_prizes_delete($table = 'prize', $criteria = array(), $limit = 0) {
  $details = _pinapi_prizes_query('prize', $criteria, '');
  $query = "DELETE FROM {pinapi_" . $table . "} prize WHERE 1 = 1" . $details['query'];
  db_query($query, $details['args']);
}

/**
 * Internal helper function constructs WHERE clauses.
 */
function _pinapi_prizes_query($table = 'prize', $criteria = array(), $alias = 'prize.') {
  $criteria += array(
    'pid' => NULL,
    'content_id' => NULL,
    'content_type' => NULL,
    'machine_name' => NULL,
    'pool_id' => NULL,
    'quantity' => NULL,
    'starts' => NULL,
    'expires' => NULL,
    'status' => NULL,
    'active' => FALSE,
  );
  
  $query = '';
  $args = array();
  if ( !empty($criteria['pid']) ) {
    _pinapi_prizes_query_builder($alias . 'pid', $criteria['pid'], $query, $args);
  }
  elseif ( !empty($criteria['active']) ) {
    _pinapi_prizes_query_builder($alias . 'active', $criteria['active'], $query, $args);
    _pinapi_prizes_query_builder($alias . 'pool_id', $criteria['pool_id'], $query, $args);
  }
  else {
    _pinapi_prizes_query_builder($alias . 'content_id', $criteria['content_id'], $query, $args);
    _pinapi_prizes_query_builder($alias . 'content_type', $criteria['content_type'], $query, $args, TRUE);
    _pinapi_prizes_query_builder($alias . 'machine_name', $criteria['machine_name'], $query, $args, TRUE);
    _pinapi_prizes_query_builder($alias . 'pool_id', $criteria['pool_id'], $query, $args);
    _pinapi_prizes_query_builder($alias . 'quantity', $criteria['quantity'], $query, $args);
    _pinapi_prizes_query_builder($alias . 'starts', $criteria['starts'], $query, $args);
    _pinapi_prizes_query_builder($alias . 'expires', $criteria['expires'], $query, $args);
    _pinapi_prizes_query_builder($alias . 'status', $criteria['status'], $query, $args);
  }
  
  return array('query' => $query, 'args' => $args);
}

/**
 * Internal helper function constructs individual elements of WHERE clauses.
 */
function _pinapi_prizes_query_builder($name, $value, &$query, &$args, $col_is_string = FALSE) {
  if (!isset($value)) {
    // Do nothing
  }
  elseif ( $name == 'prize.active' && $value ) {
    $query .= " AND prize.quantity > 0 AND prize.status = 1";
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

/**
 * Internal helper to calculate the sum of an array key value.
 * 
 * @param $array
 *   An object.
 * @param $key
 *   A key contained within the object.
 * @return
 *   The sum of the values of that key.
 */
function _pinapi_prize_pool_sum($array) {
  $total = 0;
  foreach ( $array as $a ) {
    $total += $a['quantity'] * $a['bias'];
  }
  return $total;
}
