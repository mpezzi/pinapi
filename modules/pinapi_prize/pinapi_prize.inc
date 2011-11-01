<?php 

/**
 * @file
 * Provides a generalized prize API for Drupal.
 */


/**
 * Load a Pin API Prize Object.
 *
 * @param $pid
 *   A prize id.
 * @param $reset
 *   Whether to reset the internal pinapi_prize_load cache.
 * @return
 *   A prize object.
 */
function pinapi_prize_load($pid, $reset = FALSE) {
  static $prizes = array();

  // Reset internal static cache.
  if ( $reset ) {
    $prizes = array();
  }

  // Return the statically cached prize.
  if ( isset($prizes[$pid]) ) {
    return is_object($prizes[$pid]) ? drupal_clone($prizes[$pid]) : $prizes[$pid];
  }

  // Retrieve the prize object.
  $prize = db_fetch_object(db_query("SELECT * FROM {pinapi_prize} WHERE pid = %d", $pid));

  if ( $prize && $prize->pid ) {
    // Invoke hook_pinapi_prizeapi operatioon in all module and attach results to the prize object.
    if ( $extra = pinapi_prize_prizeapi_invoke('load', $prize) ) {
      foreach ( $extra as $key => $value ) {
        $prize->$key = $value;
      }
    }

    // Store prize object to the internal static cache.
    $prizes[$prize->pid] = is_object($prize) ? drupal_clone($prize) : $prize;
  }

  return $prize;
}

/**
 * Save a Pin API Group Object.
 *
 * @param $prize
 *   A prize object.
 */
function pinapi_prize_save(&$prize) {
  // Invoke hook_pinapi_groupapi operation in all modules.
  pinapi_prize_prizeapi_invoke('presave', $prize);

  // Determine whether this is a new group.
  $prize->is_new = empty($prize->pid);

  if ( $prize->is_new ) {
    drupal_write_record('pinapi_prize', $prize);
    $op = 'insert';
  }
  else {
    drupal_write_record('pinapi_prize', $prize, 'pid');
    $op = 'update';
  }

  // Invoke hook_pinapi_groupapi operation in all modules.
  pinapi_prize_prizeapi_invoke($op, $prize);

  // Clear the page and blocks cache.
  cache_clear_all();
}

/**
 * Delete a Pin API Group Object.
 */
function pinapi_prize_delete($prize) {
  // Invoke hook_pinapi_prizeapi operation in all modules.
  pinapi_prize_prizeapi_invoke('delete', $prize);

  // Delete group information.
  db_query("DELETE FROM {pinapi_prize} WHERE pid = %d", $prize->pid);
  db_query("DELETE FROM {pinapi_prize_redeemed} WHERE pid = %d", $prize->pid);
}

/**
 * Return an array of prizes.
 *
 * @param $param
 *   Either the nid of the node or an array of conditions to match against in the database query
 * @param $reset
 *   Whether to reset the internal pinapi_prize_get_prizes cache.
 */
function pinapi_prize_get_prizes($param = array(), $reset = FALSE) {
  $cond = array();
  $arguments = array();
  $prizes = array();

  // @TODO: remove status clause
  if ( !isset($param['status']) ) {
    //$param['status'] = 1;
  }

  if ( is_array($param) && count($param) > 0 ) {
    // Turn the conditions into a query.
    foreach ( $param as $key => $value ) {
      $cond[] = db_escape_table($key) . " = '%s'";
      $arguments[] = $value;
    }
    $cond = implode(' AND ', $cond);
  }

  if ( count($arguments) > 0 ) {
    $result = db_query("SELECT pid FROM {pinapi_prize} WHERE " . $cond, $arguments);
  }
  else {
    $result = db_query("SELECT pid FROM {pinapi_prize}");
  }

  while ( $row = db_fetch_object($result) ) {
    $prizes[$row->pid] = pinapi_prize_load($row->pid, $reset);
  }

  return $prizes;
}

/**
 * Award a prize from a pool of prizes.
 *
 * @param $group
 *   A group.
 * @param $account
 *   A user account.
 * @return
 *   The awarded prize or FALSE
 */
function pinapi_prize_award($group, $account = NULL) {
  $prize = pinapi_prize_random($group, $account);

  if ( $prize && $prize->quantity > 0 ) {
    $prize->redeemable = TRUE;

    // Allow other modules to alter the prize before it is awarded.
    pinapi_prize_prizeapi_invoke('pre award', $prize, $group, $account);

    if ( $prize->redeemable ) {
      db_query("UPDATE {pinapi_prize} SET quantity = quantity - 1 WHERE pid = %d", $prize->pid);
      db_query("UPDATE {pinapi_prize} SET status = 0 WHERE quantity = 0");
    }

    // Allow other modules to alter the prize after it is awarded.
    pinapi_prize_prizeapi_invoke('award', $prize, $group, $account);

    return $prize;
  }

  return FALSE;
}

/**
 * Choose a random prize from an array of prizes based on quantities.
 *
 * @param $group
 *   A group.
 * @param $account
 *   A user account
 * @return
 *   A random prize object.
 */
function pinapi_prize_random($group, $account = NULL) {
  $prizes = pinapi_prize_get_prizes(array('status' => 1, 'gid' => $group->gid), TRUE);

  // Calculate a random number based on available prize quantities.
  $rand = rand(0, pinapi_prize_group_sum($prizes) - 1);

  // Sum up the totals and look for an appropriate match.
  $sum = 0;
  foreach ( $prizes as $prize ) {
    $sum += $prize->quantity * $prize->bias;
    if ( $rand < $sum ) {
      // Invoke hook_pinapi_groupapi operation in all modules, allow modules to alter chosen prize.
      pinapi_prize_prizeapi_invoke('random', $prize, $group, $account);
      return $prize;
    }
  }

  return FALSE;
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
function pinapi_prize_group_sum($prizes) {
  $sum = 0;
  foreach ( $prizes as $a ) {
    $sum += $a->quantity * $a->bias;
  }
  return $sum;
}

/**
 * Invoke a hook_pinapi_prizeapi operation in all modules.
 */
function pinapi_prize_prizeapi_invoke($op, &$prize, $group = NULL, $account = NULL) {
  $return = array();
  foreach ( module_implements('pinapi_prizeapi') as $name ) {
    $function = $name . '_pinapi_prizeapi';
    $result = $function($op, $prize, $group, $account);
    if ( isset($result) && is_array($result) ) {
      $return = array_merge($return, $result);
    }
    else {
      $return[] = $result;
    }
  }
  return $return;
}

/**
 * Load a Redeemed Prize Object.
 *
 * @param $rid
 *   The redeem id.
 */
function pinapi_prize_redeemed_load($rid, $reset = FALSE) {
  static $prizes_redeemed;

  if ( !isset($prizes_redeemed[$rid]) || $reset ) {
    $prize_redeemed = db_fetch_object(db_query("SELECT * FROM {pinapi_prize_redeemed} pr INNER JOIN {pinapi_code_redeemed} cr ON pr.rid = cr.rid WHERE cr.rid = %d", $rid));

    // Load associated objects and append to redeemed prize.
    if ( $prize_redeemed ) {
      $prize_redeemed->user = user_load($prize_redeemed->uid);
      $prize_redeemed->prize = pinapi_prize_load($prize_redeemed->pid);
      $prize_redeemed->group = pinapi_group_load($prize_redeemed->gid);
    }

    $prizes_redeemed[$rid] = $prize_redeemed;
  }

  return $prizes_redeemed[$rid];
}























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

