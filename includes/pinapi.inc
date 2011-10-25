<?php 

/**
 * @file
 * Provide internal helper functions for Pin API module.
 */

/**
 * Redeem a code and award a prize.
 *
 * @param $string
 *   A string to use as code.
 * @param $group
 *   A group object.
 * @param $account
 *   A user account or user id.
 * @return
 *   A prize array.
 */
function pinapi_redeem($string, $group, $account = NULL) {
  // If uid is passed load the user object.
  if ( is_numeric($account) ) {
    $account = user_load($account);
  }

  // If no account is supplied, use the anonymous user.
  if ( is_null($account) || !$account ) {
    $account = user_load(0);
  }

  // @TODO: Clean up logic.
  // Ensure code is valid and only redeem if prize can be awarded.
  if ( pinapi_code_redeem($string, $group, $account) ) {
    return TRUE;
  }

  return FALSE;
}

/**
 * Invoke hook operations.
 *
 * @param $hook
 *   Hook to be invoked.
 * @param $object.
 *   An object to send to hooks.
 */
function _pinapi_invoke($hook, &$object, $a3 = NULL, $a4 = NULL) {
  foreach ( module_implements('pinapi_' . $hook) as $module ) {
    $function = $module . '_pinapi_' . $hook;
    $function($object, $a3, $a4);
  }
}
