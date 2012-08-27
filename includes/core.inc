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
 * Pin is early.
 */
define('PINAPI_STATUS_EARLY', 4);

/**
 * Pin is late.
 */
define('PINAPI_STATUS_LATE', 5);

/**
 * Pin was not found.
 */
define('PINAPI_STATUS_NOT_FOUND', 6);

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
