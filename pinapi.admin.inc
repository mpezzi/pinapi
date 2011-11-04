<?php 

/**
 * @file
 * Configuration forms and helper functions for PinAPI module.
 */


/**
 * Administrative settings for PinAPI.
 */
function pinapi_settings_form($form_state) {
  $form = array();

  return system_settings_form($form);
}
