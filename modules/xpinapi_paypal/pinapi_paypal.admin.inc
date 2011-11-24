<?php 

/**
 * @file
 * Provides Pin API Paypal Administrative Forms.
 */

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
    '#title' => t('Business (Vendor) Email'),
    '#required' => TRUE,
    '#default_value' => variable_get('pinapi_paypal_business_email', ''),
  );
  $form['pinapi_paypal_item_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Item Name (displayed in PayPal purchase)'),
    '#required' => TRUE,
    '#default_value' => variable_get('pinapi_paypal_item_name', ''),
  );
  $form['pinapi_paypal_item_amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Item Price (displayed in PayPal purchase)'),
    '#required' => TRUE,
    '#default_value' => variable_get('pinapi_paypal_item_amount', ''),
  );
  $form['pinapi_paypal_item_number'] = array(
    '#type' => 'textfield',
    '#title' => t('Item ID Number (optional, displayed in PayPal purchase)'),
    '#default_value' => variable_get('pinapi_paypal_item_number', ''),
  );
  $form['pinapi_paypal_shipping_rate'] = array(
    '#type' => 'textfield',
    '#title' => t('Shipping Rate'),
    '#default_value' => variable_get('pinapi_paypal_shipping_rate', ''),
  );
  $form['pinapi_paypal_cancel_return_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Cancel Return Path'),
    '#required' => TRUE,
    '#default_value' => variable_get('pinapi_paypal_cancel_return_path', ''),
  );
  $form['pinapi_paypal_return_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Return Path'),
    '#required' => TRUE,
    '#default_value' => variable_get('pinapi_paypal_return_path', ''),
  );  
  $form['pinapi_paypal_notify_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Notify Path'),
    '#required' => TRUE,
    '#default_value' => variable_get('pinapi_paypal_notify_path', ''),
  );
  $form['pinapi_paypal_currency_code'] = array(
    '#type' => 'select',
    '#title' => t('Currency'),
    '#options' => pinapi_paypal_currency_codes(),
    '#default_value' => variable_get('pinapi_paypal_currency_code', ''),
  );  
  $form['pinapi_paypal_language_code'] = array(
    '#type' => 'select',
    '#title' => t('Language Code'),
    '#options' => pinapi_paypal_language_codes(),
    '#default_value' => variable_get('pinapi_paypal_language_code', ''),
  );
  return system_settings_form($form);
}