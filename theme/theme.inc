<?php

/**
 * @file
 * Provides theme functions for pinapi.module
 */


/**
 * Process variables for pinapi-redeem-form.tpl.php
 *
 * The $vars array contains the following arguments:
 * - $form
 *
 * @see pinapi-redeem-form.tpl.php
 */
function template_preprocess_pinapi_redeem_form(&$vars) {
  $vars['group'] = pinapi_group_load($vars['form']['gid']['#value']);

  // Provide varables named after form keys so themers can print each element independently.
  foreach ( element_children($vars['form']) as $key ) {
    $type = $vars['form'][$key]['#type'];
    if ( $type == 'hidden' || $type == 'token' ) {
      $hidden[] = drupal_render($vars['form'][$key]);
    }
    else {
      $vars['pinapi_redeem'][$key] = drupal_render($vars['form'][$key]);
    }
  }

  // Hidden form elements have no value to themers. No need for separation.
  $vars['pinapi_redeem']['hidden'] = implode($hidden);

  // Collect all form elements to make it easier to print the whole form.
  $vars['pinapi_redeem_form'] = implode($vars['pinapi_redeem']);
}
