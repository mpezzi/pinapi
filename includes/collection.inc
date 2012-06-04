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
  }
  return $collections[$cid];
}

/**
 * Save changes to a collection or a new collection.
 */
function pinapi_collection_save($collection) {
  $transaction = db_transaction();
  try {
    $collection->is_new = empty($collection->cid);

    if ( $collection->is_new ) {
      $result = drupal_write_record('pinapi_collection', $collection);
    }
    else {
      $result = drupal_write_record('pinapi_collection', $collection, 'cid');
    }

    pinapi_collection_static_reset();

    // Ignore slave server temporarily to give time for the saved collection to be propagated to the slave server.
    db_ignore_slave();

    return $result;
  }
  catch ( Exception $e ) {
    $transaction->rollback();
    watchlog_exception('seed', $e);
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
      db_delete('pinapi_collection')
        ->condition('cid', $collection->cid)
        ->execute();

      // Clear caches.
      pinapi_collection_static_reset();
      cache_clear_all();

      return SAVED_DELETED;
    }
    catch ( Exception $e ) {
      $transaction->rollback();
      watchlog_exception('pinapi', $e);
      throw $e;
    }
  }
}

/**
 * Reset statically cache variables.
 */
function pinapi_collection_static_reset() {
  drupal_static_reset('pinapi_collection_load');
}
