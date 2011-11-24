<?php

/**
 * @file
 * Provide internal functions for pinapi_paypal.module
 */

/**
 * Simplified PINAPI Paypal user PayPal payment status constants
 */
define('PINAPI_PAYPAL_INCOMPLETE', 0);  // user has not initiated any PayPal payment
define('PINAPI_PAYPAL_COMPLETE', 1);    // user has completed a PayPal payment
define('PINAPI_PAYPAL_PENDING', 2);     // user has a pending PayPal payment
define('PINAPI_PAYPAL_FAILED', 3);      // user's PayPal transaction returned status such as 'Denied', 'Failed', 'Expired', etc.

/**
 * Load a PayPal Order.
 */
function pinapi_paypal_order_load($txnid) {
  return db_fetch_object(db_query("SELECT * FROM {pinapi_paypal_order} WHERE txnid = '%s'", $txnid));
}

/**
 * Save a PayPal Order.
 */
function pinapi_paypal_order_save($order) {
  drupal_write_record('pinapi_paypal_order', $order, 'txnid');

  // If update did not change any rows insert the new order.
  if ( db_affected_rows() == 0 ) {
    drupal_write_record('pinapi_paypal_order', $order);
  }
}

/**
 * Builds a PayPal payment form.
 */
function pinapi_paypal_payment_form($form_state, $form = array(), $data = array()) {
  // Set form attributes required by PayPal.
  $form['#method'] = 'get';
  $form['#action'] = variable_get('pinapi_paypal_sandbox', TRUE) ?
    'https://www.sandbox.paypal.com/cgi-bin/webscr' :
    'https://www.paypal.com/cgi-bin/webscr';

  // Set any default settings if they were not set already.
  $data += array(
    'business' => variable_get('pinapi_paypal_receiver_email', ''),
    'currency_code' => variable_get('pinapi_paypal_currency_code', ''),
    'lc' => pinapi_paypal_lc(),
    'cancel_return' => url('<front>', array('absolute' => TRUE)),
    'notify_url' => url('pinapi/paypal/ipn', array('absolute' => TRUE)),
    'return' => url('<front>', array('absolute' => TRUE)),
    'upload' => 1,
    'page_style' => 'PayPal',
    'bn' => 'BuyNow',
    'cmd' => '_cart',
  );

  // Process shopping cart items and remove from data array.
  foreach ( $data['items'] as $key => $item ) {
    $i = $key + 1;

    $form['pinapi_paypal']['item_name_' . $i] = array(
      '#type' => 'hidden',
      '#value' => $item['name'],
    );
    $form['pinapi_paypal']['amount_' . $i] = array(
      '#type' => 'hidden',
      '#value' => $item['amount'],
    );
    $form['pinapi_paypal']['quantity_' . $i] = array(
      '#type' => 'hidden',
      '#value' => $item['quantity'],
    );
  }

  // We have already processed this data, remove it.
  unset($data['items']);

  // Process data as hidden input form items.
  foreach ( $data as $key => $value ) {
    $form['pinapi_paypal'][$key] = array(
      '#type' => 'hidden',
      '#value' => $value,
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Returns a status code based on a supplied PayPal payment status
 *
 * @param
 *   a payment_status string from a PayPal IPN
 *
 * @return
 *  a status code
 */
function pinapi_paypal_status_code($status) {
  switch ( $status ) {
    case 'Completed':
    case 'Processed':
    case 'Canceled_Reversal':
      return PINAPI_PAYPAL_COMPLETE;

    case 'Pending':
      return PINAPI_PAYPAL_PENDING;

    default:
      return PINAPI_PAYPAL_FAILED;
  }
}

/**
 * Define a PayPal locale
 */
function pinapi_paypal_lc($lang = NULL) {
  global $language;

  // if a language was not sent, use current language
  if ( is_null($lang) ) {
    $lang = $language->language;
  }

  $paypal_lc = array(
    'en' => 'CA',
    'fr' => 'FR',
  );

  // if paypal locale exists, return it, otherwise return English as default
  return isset($paypal_lc[$lang]) ? $paypal_lc[$lang] : $paypal_lc['en'];
}