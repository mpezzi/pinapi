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
 * Insert or update a PayPal transaction.
 */
function pinapi_paypal_save(&$paypal) {
  $paypal->is_new = empty($paypal->uid);

  if ( $paypal->is_new ) {
    drupal_write_record('pinapi_paypal', $paypal);
  }
  else {
    drupal_write_record('pinapi_paypal', $paypal, 'bid');
  }
}