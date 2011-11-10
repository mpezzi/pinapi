<?php

/**
 * @file
 * Provides Bonus PIN integration.
 */

/**
 * Load a bonus code.
 */
function pinapi_bonus_load($bid, $reset = FALSE) {
  static $bonus;

  if ( !isset($bonus[$bid]) || $reset ) {
    $bonus[$bid] = db_fetch_object(db_query("SELECT * FROM {pinapi_bonus} WHERE bid = %d", $bid));
  }

  return $bonus[$bid];
}

/**
 * Insert or update a bonus code.
 */
function pinapi_bonus_save(&$bonus) {
  $bonus->is_new = empty($bonus->bid);

  if ( $bonus->is_new ) {
    drupal_write_record('pinapi_bonus', $bonus);
  }
  else {
    drupal_write_record('pinapi_bonus', $bonus, 'bid');
  }
}

/**
 * Award bonus code to a user account.
 *
 * @param $group
 *   A group object.
 * @param $account
 *   A user object.
 * @return
 *   A code object or FALSE.
 */
function pinapi_bonus_award($group, $account, $identifier) {
  // Ensure identifier has not been used before.
  $sql  = "SELECT COUNT(*) FROM {pinapi_bonus} pb ";
  $sql .= "LEFT JOIN {pinapi_code} pc ON pb.cid = pc.cid ";
  $sql .= "WHERE pc.gid = %d AND pb.identifier = '%s' LIMIT 1";
  $used = (bool) db_result(db_query($sql, $group->gid, $identifier));

  dpm($used);

  if ( !$used ) {
    // Look for a bonus code to award.
    $sql  = "SELECT pb.*, pc.code FROM {pinapi_bonus} pb ";
    $sql .= "LEFT JOIN {pinapi_code} pc ON pb.cid = pc.cid ";
    $sql .= "WHERE pc.gid = %d AND pb.status = 1 LIMIT 1";
    $bonus = db_fetch_object(db_query($sql, $group->gid));

    // Mark as awarded.
    if ( $bonus ) {
      $bonus->uid = $account->uid;
      $bonus->identifier = $identifier;
      $bonus->status = FALSE;

      pinapi_bonus_save($bonus);

      return $bonus;
    }
  }

  return FALSE;
}
