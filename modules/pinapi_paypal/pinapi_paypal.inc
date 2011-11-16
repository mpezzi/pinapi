<?php

/**
 * @file
 * Provides simple PayPal integration.
 */

/**
 * Returns an array containing the environments that PINAPI PayPal uses.
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
 * Returns the current environment PINAPI PayPal is using.
 * @return
 *   the current PINAPI PayPal environment if set, defaults to Development Environment
 */
function pinapi_paypal_environment() {
  $environments = pinapi_paypal_environments();
  return $environments[variable_get('pinapi_paypal_environment', 1)];
}

/**
 * Returns an array containing the names of PINAPI PayPal environments 
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
 * Verifies an incoming IPN transaction's details.
 *
 * Compares incoming transaction against this site's database and 
 * pinapi_paypal configuration. Must ensure that pinapi_paypal_load 
 * has been called before this function is called.
 *
 * @param
 *   the transaction ID from the transaction
 * @param
 *   the business email from the transaction
 * @param
 *   the amount from the transaction
 * @param
 *   the currency code from the transaction
 * @param
 *   the PayPal transaction object from the database to verify against
 *
 * @return
 *   TRUE if supplied transaction details match this site's pinapi_paypal
 *   configuration, FALSE otherwise.
 */
function pinapi_paypal_verify_transaction($tid, $tbusiness, $tamount, $tcurrency, &$paypal) {
  // check that this transaction id wasn't previously submitted in the database
  // or if it was, verify that the transaction id matches the tid in the record
  watchdog('pinapi_paypal', "Note tid: @tid, db_paypal_tid: @ptid", array('@tid' => $tid, '@ptid' => $paypal->tid));
  if ( (isset($paypal->is_new) && $paypal->is_new) || (isset($paypal->tid) && $paypal->tid == $tid) ) {
    // receiver's/business email must match merchant's email registered in config
    $real_email = variable_get('pinapi_paypal_business_email', null);
    if ($real_email == $tbusiness) {
      watchdog('pinapi_paypal', "Valid business email detected.");
      // verify transaction amount against amount registered in config
      $real_amount = variable_get('pinapi_paypal_item_amount', null);
      if ($real_amount == $tamount) {
        watchdog('pinapi_paypal', "Valid amount detected.");
        // verify transaction currency against currency code registered in config
        $real_currency = variable_get('pinapi_paypal_currency_code', null);
        if ($real_currency == $tcurrency) {
          watchdog('pinapi_paypal', "Valid currency code detected.");
          return TRUE;
        }
        else {
          watchdog('pinapi_paypal', "Invalid currency code detected, code: @tcurrency", array('@tcurrency' => $tcurrency, WATCHDOG_ALERT));
        }
      }
      else {
        watchdog('pinapi_paypal', "Invalid transaction amount detected, amount: @tamount", array('@tamount' => $tamount, WATCHDOG_ALERT));
      }
    }
    else {
      watchdog('pinapi_paypal', "Invalid business email submitted, business: @tbusiness" , array('@tbusiness' => $tbusiness, WATCHDOG_ALERT));
    }
  }
  else {
    watchdog('pinapi_paypal', "Duplicate or invalid transaction ID, id: @tid", array('@tid' => $tid), WATCHDOG_ALERT);
  }
  return FALSE;
}

/**
 * Simulates handling a PayPal IPN POST.
 *
 * This is a debugging function only.
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
        $account->status = PINAPI_PAYPAL_COMPLETED;
        $account->data = serialize($posted);
        pinapi_paypal_save($account);
        watchdog('pinapi_paypal', t("New transaction record created."));
      }
      else {
        // update the user's record once payment is completed
        $account->uid = $posted['custom'];
        $account->status = PINAPI_PAYPAL_COMPLETED;
        $account->data = serialize($posted);
        pinapi_paypal_save($account);
        watchdog('pinapi_paypal', t("Transaction record was updated for @uid"), array('@uid' => $account->uid));
      }
    }
    else if ( isset($posted['payment_status']) && $posted['payment_status'] == 'Pending' ) {
      // handle successful payments marked as 'PENDING' (ex. e-cheque)
      $account->uid = $posted['uid'];
      $account->status = PINAPI_PAYPAL_PENDING;
      $account->data = serialize($posted);
      pinapi_paypal_save($account);
      watchdog('pinapi_paypal', t("Received PayPal payment marked as PENDING because of '@reason': '@data'"), array('@data' => serialize($posted)),  WATCHDOG_WARNING);
    }
    print '';
  }
  else {
    // log this POST with a warning
    watchdog('pinapi_paypal', t("Received invalid transaction..."));
    // log this POST with a warning
    watchdog('pinapi_paypal', t("Flagged suspicious PayPal submission: '@data'"), array('@data' => serialize($posted)), WATCHDOG_ALERT);
  }
}