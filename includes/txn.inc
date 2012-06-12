<?php

/**
 * @file
 * tk
 */

/**
 * Load a Transaction.
 */
function pinapi_txn_load($txn_id) {
  $txns = &drupal_static(__FUNCTION__, array());

  if ( !isset($txns[$txn_id]) ) {
    $txn = db_query('SELECT * FROM {pinapi_txn} WHERE txn_id = :txn_id', array(':txn_id' => $txn_id))->fetchObject();

    $txn->pin = pinapi_pin_load($txn->pid);
    $txn->collection = pinapi_collection_load($txn->pin->cid);
    $txn->user = user_load($txn->uid);

    $txns[$txn_id] = $txn;
  }

  return $txns[$txn_id];
}

/**
 * Save a Transaction.
 */
function pinapi_txn_save(&$txn) {
  $transaction = db_transaction();
  try {
    $txn->is_new = empty($txn->txn_id);

    // Allow other modules to act when a collection is being saved.
    module_invoke_all('pinapi_txn_presave', $txn);

    if ( $txn->is_new ) {
      $result = drupal_write_record('pinapi_txn', $txn);
    }
    else {
      $result = drupal_write_record('pinapi_txn', $txn, 'txn_id');
    }

    // Clear caches.
    pinapi_txn_static_reset();
    cache_clear_all();

    // Allow other modules to act when a collection is being saved.
    module_invoke_all('pinapi_txn_save', $txn);

    // Ignore slave server temporarily to give time for the saved collection to be propagated to the slave server.
    db_ignore_slave();

    unset($txn->is_new);

    return $result;
  }
  catch ( Exception $e ) {
    $transaction->rollback();
    watchlog_exception('pinapi', $e);
    throw $e;
  }
}

/**
 * Creates a Transaction Object.
 */
function pinapi_txn($code, $cid, $uid = NULL) {
  $txn = new stdClass();

  // Load up dependant objects.
  $txn->collection = pinapi_collection_load($cid);
  $txn->pin = pinapi_pin_load(array('code' => $code, 'cid' => $cid));
  $txn->user = is_null($uid) ? user_load($GLOBALS['user']->uid) : user_load($uid);

  // Set some sweet sweet shortcut ids.
  $txn->cid = $txn->collection->cid;
  $txn->pid = isset($txn->pin->pid) ? $txn->pin->pid : FALSE;
  $txn->uid = $txn->user->uid;

  // Additional transaction information.
  $txn->hostname = ip_address();
  $txn->timestamp = time();
  $txn->changed = time();
  $txn->status = TRUE;

  // Determine txn status.
  $txn->txn_status = pinapi_txn_status($txn);

  // Determine if txn is redeemable.
  $txn->redeemable = ( $txn->txn_status == PINAPI_STATUS_AVAILABLE );
  $txn->redeemed = FALSE;

  // Invoke hook_pinapi_request() w/ 'create' operation.
  pinapi_invoke('pinapi_txn', 'create', $txn);

  return $txn;
}

/**
 * Returns a status for a Request Object.
 */
function pinapi_txn_status($txn) {
  if ( $txn->collection && $txn->collection->status ) {
    if ( $txn->pin && $txn->pin->status ) {
      $status = PINAPI_STATUS_AVAILABLE;
    }
    else if ( $txn->pin && !$txn->pin->status ) {
      $status = PINAPI_STATUS_UNAVAILABLE;
    }
    else {
      $status = PINAPI_STATUS_NOT_FOUND;
    }
  }
  else if ( $txn->collection && !$txn->collection->status ) {
    $status = PINAPI_STATUS_CLOSED;
  }
  else {
    $status = PINAPI_STATUS_NOT_FOUND;
  }

  return $status;
}

/**
 * Redeem a Transaction.
 */
function pinapi_txn_redeem(&$txn) {
  // Invoke hook_pinapi_txn() w/ 'pre redeem' operation.
  pinapi_invoke('pinapi_txn', 'pre redeem', $txn);

  if ( $txn->redeemable ) {

    // Decrease quantity on for pin, and then disable pin if quantity is 0.
    db_query("UPDATE {pinapi_pin} SET quantity = quantity - 1 WHERE pid = :pid AND cid = :cid AND quantity > 0", array(':pid' => $txn->pid, ':cid' => $txn->cid));
    db_query("UPDATE {pinapi_pin} SET status = 0 WHERE pid = :pid AND cid = :cid AND quantity = 0", array(':pid' => $txn->pid, ':cid' => $txn->cid));

    $txn->redeemed = TRUE;

    pinapi_txn_save($txn);

    // Invoke hook_pinapi_txn() w/ 'redeem' operation.
    pinapi_invoke('pinapi_txn', 'redeem', $txn);

    pinapi_txn_stats_update($txn);

    return TRUE;
  }

  // Transaction was not redeemed.
  return FALSE;
}

/**
 * Update Transaction Statistics.
 */
function pinapi_txn_stats_update($txn) {
  pinapi_txn_stats_set($txn, 'total');

  // Invoke hook_pinapi_txn() w/ 'stat update' operation.
  pinapi_invoke('pinapi_txn', 'stats update', $txn);
}

/**
 * Updated collection information.
 */
function pinapi_txn_stats_set($txn, $name, $value = 1) {
  $query = db_merge('pinapi_txn_stats')
    ->key(array('uid' => $txn->uid, 'cid' => $txn->cid, 'name' => $name))
    ->fields(array('timestamp' => time(), 'value' => $value))
    ->expression('value', 'value + :inc', array(':inc' => 1))
    ->execute();

  return $query;
}

/**
 * Return Transaction Statistic.
 */
function pinapi_txn_stats_get($txn, $name, $default = NULL) {
  $result = db_select('pinapi_txn_stats', 's')
    ->fields('s', array('value'))
    ->condition('uid', $txn->uid)
    ->condition('cid', $txn->cid)
    ->condition('name', $name)
    ->execute()
    ->fetchField();

  return $result ? $result : $default;
}

/**
 * Increment Transaction Statistic.
 */
function pinapi_txn_stats_del($txn, $name) {
  $query = db_delete('pinapi_txn_stats')
    ->condition('uid', $txn->uid)
    ->condition('cid', $txn->cid)
    ->condition('name', $name)
    ->execute();

  return $query;
}

/**
 * Reset statically cache variables.
 */
function pinapi_txn_static_reset() {
  drupal_static_reset('pinapi_txn_load');
}
