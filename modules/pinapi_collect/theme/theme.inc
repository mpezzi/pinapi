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
  if ( user_access('redeem codes') ) {
    $redeemed = $user->pinapi['redeemed'][$vars['group']->gid];

    // If the user has redeemed enough codes, display prizes.
    if ( $redeemed >= variable_get('pinapi_collect_block_required', 5) ) {
      $vars['content'] = 'OK!';
    }

    $vars['content'] = $redeemed;
  }
}
