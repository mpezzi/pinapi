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
function pinapi_bonus_redeem($group, $account) {
  // Look for a bonus code.
  $bonus = db_fetch_object(db_query("SELECT pb.* FROM {pinapi_bonus} pb LEFT JOIN {pinapi_code} pc ON pb.cid = pc.cid WHERE pc.gid = %d AND pc.status = 1 LIMIT 1", $group->gid));

  if ( $bonus ) {
    $bonus->uid = $account->uid;
    $bonus->status = FALSE;
    drupal_write_record('pinapi_bonus', $bonus, 'bid');
  }

  return FALSE;
}
