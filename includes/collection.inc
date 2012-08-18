<?php

/**
 * @file pin.inc
 * Provides collection functionality.
 */

/**
 * Load a collection object from the database.
 */
function pinapi_collection_load($cid = NULL) {
  $collections = &drupal_static(__FUNCTION__, array());
  if ( !isset($collections[$cid]) ) {
    $collections[$cid] = db_query('SELECT * FROM {pinapi_collection} WHERE cid = :cid', array('cid' => $cid))->fetchObject();

    $result = db_select('pinapi_collection_cache', 'p')
      ->fields('p', array('name', 'value'))
      ->condition('cid', $cid)
      ->execute()
      ->fetchAllAssoc('name');

    $collections[$cid]->inventory = array();
    foreach ( $result as $r ) {
      $collections[$cid]->inventory[$r->name] = $r->value;
    }
  }
  return $collections[$cid];
}

/**
 * Load all collection objects from the database.
 */
function pinapi_collection_load_all() {
  $collections = array();
  foreach ( db_query("SELECT cid FROM {pinapi_collection}") as $collection ) {
    $collections[] = pinapi_collection_load($collection->cid);
  }
  return $collections;
}

/**
 * Load a collection object by machine name from the database.
 */
function pinapi_collection_load_machine_name($name) {
  return db_query('SELECT * FROM {pinapi_collection} WHERE machine_name = :name', array('name' => $name))->fetchObject();
}

/**
 * Save changes to a collection or a new collection.
 */
function pinapi_collection_save($collection) {
  $transaction = db_transaction();
  try {
    $collection->is_new = empty($collection->cid);

    // Allow other modules to act when a collection is being saved.
    module_invoke_all('pinapi_collection_presave', $collection);

    if ( $collection->is_new ) {
      $result = drupal_write_record('pinapi_collection', $collection);
    }
    else {
      $result = drupal_write_record('pinapi_collection', $collection, 'cid');
    }

    // Clear caches.
    pinapi_collection_static_reset();
    cache_clear_all();

    // Allow other modules to act when a collection is being saved.
    module_invoke_all('pinapi_collection_save', $collection);

    // Ignore slave server temporarily to give time for the saved collection to be propagated to the slave server.
    db_ignore_slave();

    return $result;
  }
  catch ( Exception $e ) {
    $transaction->rollback();
    watchdog_exception('pinapi', $e);
    throw $e;
  }
}

/**
 * Delete a collection.
 */
function pinapi_collection_delete($cid) {
  if ( $collection = pinapi_collection_load($cid) ) {
    $transaction = db_transaction();
    try {
      // Allow other modules to act on a collection being deleted.
      module_invoke_all('pinapi_collection_delete', $collection);

      // Delete all pins and transactions for collection.
      pinapi_collection_clear($collection->cid);

      db_delete('pinapi_collection')
        ->condition('cid', $collection->cid)
        ->execute();

      // Clear caches.
      pinapi_collection_static_reset();
      cache_clear_all();

      // Ignore slave server temporarily to give time for the saved collection to be propagated to the slave server.
      db_ignore_slave();

      return SAVED_DELETED;
    }
    catch ( Exception $e ) {
      $transaction->rollback();
      watchdog_exception('pinapi', $e);
      throw $e;
    }
  }
}

/**
 * Calculate pins for a collection.
 */
function pinapi_collection_calculate($cid) {
  if ( $collection = pinapi_collection_load($cid) ) {
    // Invoke hook_pinapi_collection_calculate().
    module_invoke_all('pinapi_collection_calculate', $collection);

    // Total amount of pins for collection.
    $total = db_query('SELECT SUM(quantity_original) FROM {pinapi_pin} WHERE cid = :cid', array(':cid' => $collection->cid))
      ->fetchField();

    // Total amount of available pins for collection.
    $available = db_query('SELECT SUM(quantity) FROM {pinapi_pin} WHERE cid = :cid', array(':cid' => $collection->cid))
      ->fetchField();

    pinapi_cache_collection_set($collection->cid, 'total', $total);
    pinapi_cache_collection_set($collection->cid, 'available', $available);
  }
}

/**
 * Reset a Collection.
 *
 * Removes transactions, resets pins and cache.
 *
 * @param $cid
 *   A Collection ID.
 */
function pinapi_collection_reset($cid) {
  if ( $collection = pinapi_collection_load($cid) ) {
    // Invoke hook_pinapi_collection_reset().
    module_invoke_all('pinapi_collection_reset', $collection);

    // Delete transactions and cache.
    db_query("DELETE pt.* FROM {pinapi_txn} pt INNER JOIN {pinapi_pin} pp ON pt.pid = pp.pid WHERE pp.cid = :cid", array(':cid' => $collection->cid));
    db_query("DELETE FROM {pinapi_cache} WHERE cid = :cid", array(':cid' => $collection->cid));
    db_query("DELETE FROM {pinapi_collection_cache} WHERE cid = :cid", array(':cid' => $collection->cid));

    // Reset pins.
    // @TODO: What happens with quantities field?
    db_query("UPDATE {pinapi_pin} SET changed = 0, quantity = 1, status = 1 WHERE cid = :cid", array(':cid' => $collection->cid));

    // Clear caches.
    pinapi_collection_static_reset();
    pinapi_pin_static_reset();

    cache_clear_all();
  }
}

/**
 * Clear a Collection.
 *
 * Removes transactions, pins and cache.
 *
 * @param $cid
 *   A Collection ID.
 */
function pinapi_collection_clear($cid) {
  if ( $collection = pinapi_collection_load($cid) ) {
    // Invoke hook_pinapi_collection_clear().
    module_invoke_all('pinapi_collection_clear', $collection);

    // Delete rows from tables.
    db_query("DELETE pt.* FROM {pinapi_txn} pt INNER JOIN {pinapi_pin} pp ON pt.pid = pp.pid WHERE pp.cid = :cid", array(':cid' => $collection->cid));
    db_query("DELETE FROM {pinapi_pin} WHERE cid = :cid", array(':cid' => $collection->cid));
    db_query("DELETE FROM {pinapi_cache} WHERE cid = :cid", array(':cid' => $collection->cid));
    db_query("DELETE FROM {pinapi_collection_cache} WHERE cid = :cid", array(':cid' => $collection->cid));

    // Clear caches.
    pinapi_collection_static_reset();
    pinapi_pin_static_reset();

    cache_clear_all();
  }
}

/**
 * Reset statically cache variables.
 */
function pinapi_collection_static_reset() {
  drupal_static_reset('pinapi_collection_load');
}
