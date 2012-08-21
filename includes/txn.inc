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
    $txn->cid = $txn->pin->cid;

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

    // Clear txn static cache.
    pinapi_txn_static_reset();

    // @TODO: Do we need this?
    //cache_clear_all();

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
 * Delete a Transaction.
 */
function pinapi_txn_delete($txn_id) {
  if ( $txn = pinapi_txn_load($txn_id) ) {
    $transaction = db_transaction();
    try {
      // Allow other modules to act on a collection being deleted.
      module_invoke_all('pinapi_txn_delete', $txn);

      $result = db_delete('pinapi_txn')
        ->condition('txn_id', $txn->txn_id)
        ->execute();

      // Clear caches.
      pinapi_txn_static_reset();

      // @TODO: Do we need this?
      //cache_clear_all();

      // Update Pin API Cache Value.
      pinapi_cache_inc($txn->uid, $txn->cid, 'total', -1);

      // Ignore slave server temporarily to give time for the saved collection to be propagated to the slave server.
      db_ignore_slave();
    }
    catch ( Exception $e ) {
      $transaction->rollback();
      watchdog_exception('pinapi', $e);
      throw $e;
    }
  }

  return $result;
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
  $txn->cid = isset($txn->collection->cid) ? $txn->collection->cid : FALSE;
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

  // Invoke hook_pinapi_txn() w/ 'create' operation.
  pinapi_invoke('pinapi_txn', 'create', $txn);

  return $txn;
}

/**
 * Returns a status for a Request Object.
 */
function pinapi_txn_status($txn) {
  if ( $txn->collection && $txn->collection->status ) {
    if ( $txn->pin && $txn->pin->status ) {
      // Check if is within allowed time.
      if ( $txn->pin->start != 0 && $txn->pin->start >= $txn->timestamp ) {
        $status = PINAPI_STATUS_EARLY;
      } else if ( $txn->pin->end != 0 && $txn->pin->end < $txn->timestamp ) {
        $status = PINAPI_STATUS_LATE;
      } else {
        $status = PINAPI_STATUS_AVAILABLE;
      }
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
    db_update('pinapi_pin')
      ->fields(array('changed' => time()))
      ->expression('quantity', 'quantity - 1')
      ->condition('pid', $txn->pid)
      ->condition('cid', $txn->cid)
      ->condition('quantity', 0, '>')
      ->execute();
    db_update('pinapi_pin')
      ->fields(array('status' => 0, 'changed' => time()))
      ->condition('pid', $txn->pid)
      ->condition('cid', $txn->cid)
      ->condition('quantity', 0)
      ->execute();

    //db_query("UPDATE {pinapi_pin} SET quantity = quantity - 1, changed = :changed WHERE pid = :pid AND cid = :cid AND quantity > 0", array(':pid' => $txn->pid, ':cid' => $txn->cid));
    //db_query("UPDATE {pinapi_pin} SET status = 0 WHERE pid = :pid AND cid = :cid AND quantity = 0", array(':pid' => $txn->pid, ':cid' => $txn->cid));

    $txn->redeemed = TRUE;

    pinapi_txn_save($txn);

    // Update Pin API Cache Value.
    pinapi_cache_inc($txn->uid, $txn->cid, 'txn_total');

    // Invoke hook_pinapi_txn() w/ 'redeem' operation.
    pinapi_invoke('pinapi_txn', 'redeem', $txn);

    // Decrease collection cache counts.
    if ( !empty($txn->collection->inventory) ) {
      if ( $txn->pin->test ) {
        pinapi_cache_collection_inc($txn->cid, 'available_test', -1);
      }
      else {
        pinapi_cache_collection_inc($txn->cid, 'available', -1);
      }
    }

    return TRUE;
  }

  // Transaction was not redeemed.
  return FALSE;
}

/**
 * Calculate transactions.
 */
function pinapi_txn_calculate($name, $cid, $uid = NULL) {
  // Calculate txn_total.
  if ( $name == 'txn_total' ) {

    $sql  = "SELECT pp.cid, pt.uid, COUNT(uid) as value FROM {pinapi_txn} pt ";
    $sql .= "INNER JOIN {pinapi_pin} pp ON pt.pid = pp.pid ";
    $sql .= "WHERE pp.cid = :cid ";

    // Filter uid.
    if ( !is_null($uid) ) {
      $sql .= "AND pt.uid = :uid ";
    }

    $sql .= "GROUP BY pt.uid";

    $result = db_query($sql, array(':cid' => $cid, ':uid' => $uid));
    foreach ( $result as $row ) {
      pinapi_cache_set($row->uid, $row->cid, 'txn_total', $row->value);
    }
  }

}

/**
 * Reset statically cache variables.
 */
function pinapi_txn_static_reset() {
  drupal_static_reset('pinapi_txn_load');
}

/**
 * Returns properties of Transaction data object.
 */
function _pinapi_txn_properties() {
  return array(
    'user' => array(
      'type' => 'user',
      'label' => t('User'),
      'description' => t('The user that created the transaction'),
      'setter callback' => 'entity_metadata_verbatim_set',
    ),
  );
}