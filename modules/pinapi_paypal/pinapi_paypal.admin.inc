<?php 

/**
 * @file
 * Provides Pin API Paypal Administrative Forms.
 */

define('PAYPAL_SANDBOX_URL', 'https://www.sandbox.paypal.com/cgi-bin/webscr');
define('PAYPAL_PRODUCTION_URL', 'https://www.paypal.com/cgi-bin/webscr');
define('ENV_DEVELOPMENT', '0');
define('ENV_PRODUCTION', '1');

/**
 * Pin API Paypal Administrative Settings Form.
 */
function pinapi_paypal_settings_form() {
  $form['pinapi_paypal_environment'] = array(
    '#type' => 'select',
    '#title' => 'Environment',
    '#options' => pinapi_paypal_environment_options(),
    '#default_value' => variable_get('pinapi_paypal_environment', ''),
  );
  $form['pinapi_paypal_business_email'] = array(
    '#type' => 'textfield',
    '#title' => 'Business (Vendor) Email',
    '#default_value' => variable_get('pinapi_paypal_business_email', ''),
  );
  $form['pinapi_paypal_item_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Item Name (displayed in PayPal purchase)',
    '#required' => TRUE,
    '#default_value' => variable_get('pinapi_paypal_item_name', ''),
  );
  $form['pinapi_paypal_item_amount'] = array(
    '#type' => 'textfield',
    '#title' => 'Item Price (displayed in PayPal purchase)',
    '#required' => TRUE,
    '#default_value' => variable_get('pinapi_paypal_item_amount', ''),
  );
  $form['pinapi_paypal_item_number'] = array(
    '#type' => 'textfield',
    '#title' => 'Item ID Number (optional, displayed in PayPal purchase)',
    '#default_value' => variable_get('pinapi_paypal_item_number', ''),
  );
  $form['pinapi_paypal_shipping_rate'] = array(
    '#type' => 'textfield',
    '#title' => 'Shipping Rate',
    '#default_value' => variable_get('pinapi_paypal_shipping_rate', ''),
  );
  $form['pinapi_paypal_cancel_return_path'] = array(
    '#type' => 'textfield',
    '#title' => 'Cancel Return Path',
    '#default_value' => variable_get('pinapi_paypal_cancel_return_path', ''),
  );
  $form['pinapi_paypal_return_path'] = array(
    '#type' => 'textfield',
    '#title' => 'Return Path',
    '#default_value' => variable_get('pinapi_paypal_return_path', ''),
  );  
  $form['pinapi_paypal_notify_path'] = array(
    '#type' => 'textfield',
    '#title' => 'Notify Path',
    '#default_value' => variable_get('pinapi_paypal_notify_path', ''),
  );
  $currency_codes = array(
    'AUD' => t('Australian Dollar'),
    'GBP' => t('British Pound'),
    'CAD' => t('Canadian Dollar'),
    'CZK' => t('Czech Koruna'),
    'DKK' => t('Danish Kroner'),
    'EUR' => t('Euro'),
    'HKD' => t('Hong Kong Dollar'),
    'HUF' => t('Hungarian Forint'),
    'ILS' => t('Israeli New Shekel'),
    'JPY' => t('Japanese Yen'),
    'MXN' => t('Mexican Peso'),
    'NZD' => t('New Zealand Dollar'),
    'NOK' => t('Norwegian Kroner'),
    'PLN' => t('Polish Zlotych'),
    'SGD' => t('Singapore Dollar'),
    'SEK' => t('Swedish Kronor'),
    'CHF' => t('Swiss Franc'),
    'USD' => t('U.S. Dollar'),
  );
  $form['pinapi_paypal_currency_code'] = array(
    '#type' => 'select',
    '#title' => t('Currency'),
    '#options' => $currency_codes,
    '#default_value' => variable_get('pinapi_paypal_currency_code', ''),
  );  
  $form['pinapi_paypal_language_code'] = array(
    '#type' => 'select',
    '#title' => t('Language Code'),
    '#options' => array(
      'CA' => 'Canadian English',
    ),
    '#default_value' => variable_get('pinapi_paypal_language_code', ''),
  );
  return system_settings_form($form);
}

function pinapi_paypal_settings_form_submit($form, &$form_state) {
  // if ($form_state['values']['pinapi_paypal_environment'] === ENV_DEVELOPMENT) {
  //   variable_set('pinapi_paypal_action', 'hello');
  // } else {
  //   variable_set('pinapi_paypal_action', 'helloooo');
  // }
  // 
  dsm($form_state);
}