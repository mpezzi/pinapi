<?php

/**
 * @file
 * Provides theme functions for pinapi_collect.module
 */


/**
 * Process variables for block-pinapi-collect.tpl.php
 */
function template_preprocess_pinapi_collect_block(&$vars) {
  $account = $vars['account'];
  $group = $vars['group'];

  // Get amount of redeemed codes from selected group.
  $redeemed = isset($account->pinapi['redeemed'][$group->gid]) ? $account->pinapi['redeemed'][$group->gid] : 0;

  // Display status of how many codes were redeemed and how many are needed.
  $vars['content'] = $redeemed . '/' . variable_get('pinapi_collect_block_required', 15);

  // If we have enough pins, display a Yay! messages! Hurray!
  // @TODO: Invoke hook to allow modules to add to this, or we can modify this using a preprocess hook.
  if ( $redeemed >= variable_get('pinapi_collect_block_required', 15) ) {
    if ( module_exists('pinapi_paypal') ) {
      $vars['content'] .= drupal_get_form('pinapi_paypal_form');
    }
  }
  else {
    $vars['content'] .= drupal_get_form('pinapi_redeem_form', $group);
  }
}
