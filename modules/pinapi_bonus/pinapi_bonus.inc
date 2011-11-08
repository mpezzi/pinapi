<?php

/**
 * @file
 * Provides Bonus PIN integration.
 */

/**
 * Assigns a bonus pin to the supplied user
 * @param
 *   the user object
 */
function pinapi_bonus_assign_bonus_pin($user) {
  // retrieve the first available bonus pin
  $result = db_query("SELECT * FROM {pinapi_bonus} WHERE status = 1 LIMIT 1");
  $bonus = db_fetch_object($result);

  $data = array(
    'uid' => $user->uid,
    'bid' => $bonus->bid,
    'status' => '0',
  );

  drupal_write_record('pinapi_bonus', $data, array('bid') );
}