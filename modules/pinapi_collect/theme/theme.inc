<?php

/**
 * @file
 * Provides theme functions for pinapi_collect.module
 */


/**
 * Process variables for block-pinapi-collect.tpl.php
 */
function template_preprocess_pinapi_collect_block(&$vars) {
  global $user;

  $vars['content'] = '';

  // Only display for authenticated users.
  if ( $user->uid ) {
    // Determine the number of codes redeemed for the user.
    $redeemed = db_result(db_query("SELECT COUNT(*) FROM {pinapi_code_redeemed} WHERE uid = %d AND gid = %d AND status = 1", $user->uid, $vars['group']->gid));

    // If the user has redeemed enough codes, display prizes.
    if ( $redeemed >= variable_get('pinapi_collect_block_required', 20) ) {
      $vars['content'] = 'OK!';
    }
  }
}
