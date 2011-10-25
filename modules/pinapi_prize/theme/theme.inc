<?php 

/**
 * @file
 * Provides theme functions for pinapi_prize.module
 */

/**
 * Process variables for pinapi-prize.tpl.php
 */
function template_preprocess_pinapi_prize(&$vars) {
  $vars['content'] = drupal_render($vars['prize']['content']);
}
