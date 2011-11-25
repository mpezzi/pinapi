<?php 

/**
 * @file
 * Provides theme functions for pinapi_paypal.module
 */

/**
 * Process variables for pinapi-paypal.tpl.php
 */
function template_preprocess_pinapi_paypal(&$vars) {
  $vars['content'] = drupal_render($vars['paypal']['content']);
}
