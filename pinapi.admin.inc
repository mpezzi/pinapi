<?php 

/**
 * @file
 * Configuration forms and helper functions for PinAPI module.
 */


/**
 * Administrative settings for PinAPI.
 */
function pinapi_settings_form($form_state) {
  $form['debug'] = array(
    '#type' => 'fieldset',
    '#title' => t('Debug'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#weight' => -100,
  );
  
  $form['debug']['pinapi_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Simple Debug Mode'),
    '#description' => t('Enable debugging messages for users with "debug pin api".'),
    '#default_value' => variable_get('pinapi_debug', FALSE),
  );
  
  return system_settings_form($form);
}
