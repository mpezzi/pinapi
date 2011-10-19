<?php 

/**
 * @file
 * Provide internal helper functions for Pin API module.
 */

/**
 * Redeem a code and award a prize.
 *
 * @param $code
 *   A code
 * @param $pool_id
 *   A pool id
 * @param $account
 *   A user object to award the prize to.
 * @return
 *   A prize array.
 */
function pinapi_redeem($code, $pool_id, $account = NULL) {
  // If uid is passed load the user object.
  if ( is_numeric($account) ) {
    $account = user_load($account);
  }
  
  // If no account is supplied, use the anonymous user.
  if ( is_null($account) || !$account ) {
    $account = user_load(0);
  }
  
  // Ensure code is valid and only redeem if prize can be awarded.
  if ( pinapi_code_verify($code, $pool_id, $account) ) {
    if ( $prize = pinapi_prize_award($pool_id, $account) ) {
      if ( pinapi_code_redeem($code, $pool_id, $prize, $account) ) {
        return $prize;
      }
    }
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
