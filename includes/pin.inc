<?php

/**
 * @file pin.inc
 * Provides pin functionality.
 */


/**
 * Load a pin.
 */
function pinapi_pin_load($param) {
  $pins = &drupal_static(__FUNCTION__, array());

  $conds = array();
  if ( is_numeric($param) ) {
    // Pin is statically cached.
    if ( isset($pins[$param]) ) {
      return $pins[$param];
    }
    $conds['pid'] = $param;
  }
  elseif ( is_array($param) ) {
    foreach ( $param as $key => $value ) {
      $conds[$key] = $value;
    }
  }
  else {
    return FALSE;
  }

  $query = db_select('pinapi_pin', 'p')
    ->fields('p', array());

  foreach ( $conds as $field => $value ) {
    $query->condition($field, $value);
  }

  $pin = $query->range(0, 1)->execute()->fetchObject();

  if ( $pin && $pin->pid ) {
    $pins[$pin->pid] = $pin;
  }

  return $pin;
}

/**
 * Save a pin.
 */
function pinapi_pin_save($pin) {
  $transaction = db_transaction();
  try {
    $pin->is_new = empty($pin->pid);

    if ( $pin->is_new ) {
      $result = drupal_write_record('pinapi_pin', $pin);
    }
    else {
      $result = drupal_write_record('pinapi_pin', $pin, 'pid');
    }

    // Clear caches.
    pinapi_pin_static_reset();

    // Allow other modules to act when a collection is being saved.
    module_invoke_all('pinapi_pin_save', $pin);

    // Ignore slave server temporarily to give time for the saved collection to be propagated to the slave server.
    db_ignore_slave();

    return $result;
  }
  catch ( Exception $e ) {
    $transaction->rollback();
    watchdog_exception('pinapi', $e);
    throw $e;
  }

  return $result;
}

/**
 * Redeem a pin.
 */
function pinapi_pin_redeem($string, $collection, $account = NULL) {

}

/**
 * Returns the status of a pin.
 */
function pinapi_pin_status($pin, $account = NULL) {

}

/**
 * Reset pins.
 */
function pinapi_pin_reset() {

}

/**
 * Reset statically cache variables.
 */
function pinapi_pin_static_reset() {
  drupal_static_reset('pinapi_pin_load');
}
