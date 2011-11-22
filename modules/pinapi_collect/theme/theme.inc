<?php

/**
 * @file
 * Provides theme functions for pinapi_collect.module
 */


/**
 * Process variables for pinapi-collect.tpl.php
 */
function template_preprocess_pinapi_collect(&$vars) {
  $account = $vars['account'];
  $group = $vars['group'];

  // Number of codes redeemed and needed.
  $vars['redeemed'] = isset($account->pinapi['redeemed'][$group->gid]) ? $account->pinapi['redeemed'][$group->gid] : 0;
  $vars['needed'] = variable_get('pinapi_collect_' . $group->gid . '_codes_needed', 15);

  // Pin API Redeem form.
  $vars['redeem'] = drupal_get_form('pinapi_redeem_form', $group);
}
