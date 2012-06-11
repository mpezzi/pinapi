<?php

/**
 * @file core.inc
 * Provides core functionality.
 */

/**
 * Pin is available.
 */
define('PINAPI_STATUS_AVAILABLE', 1);

/**
 * Pin is unavailable.
 */
define('PINAPI_STATUS_UNAVAILABLE', 2);

/**
 * Pin collection is closed.
 */
define('PINAPI_STATUS_CLOSED', 3);

/**
 * Pin was not found.
 */
define('PINAPI_STATUS_NOT_FOUND', 4);


/**
 * Invoke hook_pinapi operation in all modules.
 */
function pinapi_invoke($hook, $op, &$object) {
  $return = array();

  foreach ( module_implements($hook) as $name ) {
    $function = $name . '_' . $hook;
    $result = $function($op, $object);
    if ( isset($result) && is_array($result) ) {
      $return = array_merge($return, $result);
    }
    else if (isset($result)) {
      $return[] = $result;
    }
  }

  return $return;
}

/**
 * Generate Test Object.
 */
function pinapi_test_generate() {
  $collection = (object) array(
    'name' => t('Test'),
    'machine_name' => 'test',
    'status' => 1,
  );
  pinapi_collection_save($collection);

  for ( $i = 0; $i < 5; $i++ ) {
    $pin = (object) array(
      'code' => 'test' . $i,
      'cid' => $collection->cid,
      'quantity' => 1,
      'status' => 1,
    );
    pinapi_pin_save($pin);
  }
}