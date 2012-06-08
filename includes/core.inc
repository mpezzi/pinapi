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
 * Pin was not found.
 */
define('PINAPI_STATUS_NOT_FOUND', 3);

/**
 * Pin collection is closed.
 */
define('PINAPI_STATUS_CLOSED', 4);

/**
 * Returns a Pin API Request Object.
 */
function pinapi_request($code, $cid, $uid = NULL) {
  global $user;

  $request = new stdClass();
  $request->collection = pinapi_collection_load($cid);
  $request->pin = pinapi_pin_load(array('code' => $code, 'cid' => $cid));

  // Append requesting account.
  if ( !is_null($uid) ) {
    $request->account = user_load($uid);
  }
  else {
    $request->account = user_load($user->uid);
  }

  // Determine request status.
  if ( $request->collection && $request->collection->status ) {
    if ( $request->pin && $request->pin->status ) {
      $request->status = PINAPI_STATUS_AVAILABLE;
    }
    else if ( $request->pin && !$request->pin->status ) {
      $request->status = PINAPI_STATUS_UNAVAILABLE;
    }
    else {
      $request->status = PINAPI_STATUS_NOT_FOUND;
    }
  }
  else if ( $request->collection && !$request->collection->status ) {
    $request->status = PINAPI_STATUS_CLOSED;
  }
  else {
    $request->status = PINAPI_STATUS_NOT_FOUND;
  }

  // Invoke load hook in other modules.
  pinapi_invoke('pinapi', 'load', $request);

  // Set as redeemable by default.
  if ( !isset($request->redeemable) ) {
    $request->redeemable = TRUE;
  }

  return $request;
}

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