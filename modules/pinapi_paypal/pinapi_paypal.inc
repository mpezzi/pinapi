<?php

/**
 * @file
 * Provides simple PayPal integration.
 */

/**
 * Returns an array containing the environments that Pinapi PayPal uses.
 */
function pinapi_paypal_environments() {
  return array(
    PINAPI_PAYPAL_ENV_DEVELOPMENT => array(
      'name' => 'Development',
      'action' => 'https://www.sandbox.paypal.com/cgi-bin/webscr',
    ),
    PINAPI_PAYPAL_ENV_PRODUCTION => array(
      'name' => 'Production',
      'action' => 'https://www.paypal.com/cgi-bin/webscr',
    ),
  );
}

/**
 * Returns the current environment Pinapi PayPal is using.
 * @return
 *   the current Pinapi PayPal environment if set, defaults to Development Environment
 */
function pinapi_paypal_environment() {
  $environments = pinapi_paypal_environments();
  return $environments[variable_get('pinapi_paypal_environment', 1)];
}

/**
 * Returns an array containing the names of Pinapi PayPal environments 
 */
function pinapi_paypal_environment_options() {
  $options = array();
  foreach ( pinapi_paypal_environments() as $key => $environment ) {
    $options[$key] = $environment['name'];
  }
  return $options;
}

function pinapi_paypal_currency_codes() {
  return array(
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
}

/**
 * Returns an array of language codes supported by PayPal
 * @see Appendice C in https://cms.paypal.com/cms_content/US/en_US/files/developer/PP_WebsitePaymentsStandard_IntegrationGuide.pdf
 */
function pinapi_paypal_language_codes() {
  return array(
    'AU'	=> t('Australia'),
    'AT'	=> t('Austria'),
    'BE'	=> t('Belgium'),
    'CA'	=> t('Canada'),
    'CN'	=> t('China'),
    'FR'	=> t('France'),
    'DE'	=> t('Germany'),
    'HK'	=> t('Hong Kong'),
    'IT'	=> t('Italy'),
    'MX'	=> t('Mexico'),
    'NL'	=> t('Netherlands'),
    'NZ'	=> t('New Zealand'),
    'PL'	=> t('Poland'),
    'SG'	=> t('Singapore'),
    'ES'	=> t('Spain'),
    'SE'	=> t('Sweden'),
    'CH'	=> t('Switzerland'),
    'GB'	=> t('United Kingdom'),
    'US'	=> t('United States'),
  );
}

function pinapi_paypal_user_redeemed() {
  dpm($GLOBALS['user']);
}

/**
 * Load a user's PayPal transaction record.
 */
function pinapi_paypal_load($uid, $reset = FALSE) {
  static $paypal;

  if ( !isset($paypal[$uid]) || $reset ) {
    $paypal[$uid] = db_fetch_object(db_query("SELECT * FROM {pinapi_paypal} WHERE uid = %d", $uid));
  }

  return $paypal[$uid];
}

/**
 * Insert or update a PayPal transaction record.
 */
function pinapi_paypal_save(&$paypal) {
  if ( isset($paypal->is_new) && $paypal->is_new ) {
    drupal_write_record('pinapi_paypal', $paypal);
  }
  else {
    drupal_write_record('pinapi_paypal', $paypal, 'uid');
  }
}

/**
 * Checks the details of a transaction to verify its origin
 * and details are truly from this site
 *
 * @param
 *   the transaction ID from the transaction
 * @param
 *   the business email the transaction
 * @param
 *   the amount of the transaction
 */
function pinapi_verify_transaction($tid, $tbusiness, $tamount) {
  $query = db_fetch_object(db_query("SELECT * FROM {pinapi_paypal} WHERE tid = %d", $tid));
  // check that this transaction id exists in the database
  if ($query) {
    // receiver's email/Business must match the registered merchant's email (receiver_email/business)
    $real_email = variable_get('pinapi_paypal_business_email', null);
    if ($real_email == $tbusiness) {
      // verify transaction amount against amount registered in system
      $real_amount = variable_get('pinapi_paypal_item_amount', null);
      if ($real_amount == $tamount) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

/**
 * A debugging function that simulates handling a PayPal IPN POST
 */
function pinapi_paypal_verify() {
  // generate a simulated POST using previously POSTed paypal data
  $account = pinapi_paypal_load(1);
  $posted = json_decode($account->data, TRUE);
  $query = array();
  $query['cmd'] = '_notify-validate';
  $query = array_merge($query, $posted);
  $data = drupal_query_string_encode($query);

  // send request to PayPal to verify transaction being referenced
  $environment = pinapi_paypal_environment();
  $response = drupal_http_request($environment['action'], array(), 'POST', $data, 3);
  watchdog('pinapi_paypal', t("Verification response received: '@data'"), array('@data' => $response->data));

  // only process this POST if verified by PayPal
  if ( $response->data == 'VERIFIED' ) {
    watchdog('pinapi_paypal', t("Received verified transaction."));
    if ( isset($posted['payment_status']) && $posted['payment_status'] == 'Completed' ) {
      if ( !$account ) {
        // create a new transaction record
        $account->status = PINAPI_PAYPAL_PAYMENT_COMPLETED;
        $account->data = serialize($posted);
        pinapi_paypal_save($account);
        watchdog('pinapi_paypal', t("New transaction record created."));
      } else {
        // update the user's record once payment is completed
        $account->uid = $posted['custom'];
        $account->status = PINAPI_PAYPAL_PAYMENT_COMPLETED;
        $account->data = serialize($posted);
        pinapi_paypal_save($account);
        watchdog('pinapi_paypal', t("Transaction record was updated for @uid"), array('@uid' => $account->uid));
      }
    } else if ( isset($posted['payment_status']) && $posted['payment_status'] == 'Pending' ) {
      // handle successful payments marked as 'PENDING' (ex. e-cheque)
      $account->uid = $posted['uid'];
      $account->status = PINAPI_PAYPAL_PAYMENT_PENDING;
      $account->data = serialize($posted);
      pinapi_paypal_save($account);
      watchdog('pinapi_paypal', t("Received PayPal payment marked as PENDING because of '@reason': '@data'"), array('@data' => serialize($posted)),  WATCHDOG_WARNING);
    }
    print '';
  } else {
    // log this POST with a warning
    watchdog('pinapi_paypal', t("Received invalid transaction..."));
    // log this POST with a warning
    watchdog('pinapi_paypal', t("Flagged suspicious PayPal submission: '@data'"), array('@data' => serialize($posted)), WATCHDOG_ALERT);
  }
}