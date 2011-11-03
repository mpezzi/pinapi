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