<?php 

/**
 * @file
 * Provides theme functions for pinapi.module
 */

/**
 * Process variables for prize.tpl.php
 */
function template_preprocess_prize(&$vars) {
  $vars['content'] = drupal_render($vars['prize']['content']);
}
