<?php 

/**
 * @file
 * Provides Pin API Paypal Administrative Forms.
 */

/**
 * Pin API Paypal Administrative Settings Form.
 */
function pinapi_bonus_settings_form() {
  $form['pinapi_bonus_environment'] = array(
    '#type' => 'select',
    '#title' => 'Environment',
    '#options' => pinapi_bonus_environment_options(),
    '#default_value' => variable_get('pinapi_bonus_environment', ''),
  );
  return system_settings_form($form);
}