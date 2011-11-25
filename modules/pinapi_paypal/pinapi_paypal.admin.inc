<?php 

/**
 * @file
 * Provides Pin API Paypal Administrative Forms.
 */

/**
 * Pin API Paypal Administrative Settings Form.
 */
function pinapi_paypal_settings_form() {
  $form['pinapi_paypal_receiver_email'] = array(
    '#type' => 'textfield',
    '#title' => t('Business (Vendor) Email'),
    '#required' => TRUE,
    '#default_value' => variable_get('pinapi_paypal_receiver_email', ''),
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
  $form['pinapi_paypal_country_code'] = array(
    '#type' => 'select',
    '#title' => t('Country Code'),
    '#options' => pinapi_paypal_country_codes(),
    '#default_value' => variable_get('pinapi_paypal_country_code', ''),
  );
  return system_settings_form($form);
}

/**
 * Returns an array of currency codes supported by PayPal
 * @see also Appendice D in https://cms.paypal.com/cms_content/US/en_US/files/developer/PP_WebsitePaymentsStandard_IntegrationGuide.pdf
 */
function pinapi_paypal_currency_codes() {
  return array(
    'USD' => t('U.S. Dollar'),
    'CAD' => t('Canadian Dollar'),
    'GBP' => t('British Pound'),
    'AUD' => t('Australian Dollar'),
    'BRL' => t('Brazilian Real*'),
    'CZK' => t('Czech Koruna'),
    'DKK' => t('Danish Kroner'),
    'EUR' => t('Euro'),
    'HKD' => t('Hong Kong Dollar'),
    'HUF' => t('Hungarian Forint'),
    'ILS' => t('Israeli New Shekel'),
    'JPY' => t('Japanese Yen'),
    'MYR' => t('Malaysian Ringgit*'),
    'MXN' => t('Mexican Peso'),
    'NOK' => t('Norwegian Kroner'),
    'NZD' => t('New Zealand Dollar'),
    'PHP' => t('Philippine Peso'),
    'PLN' => t('Polish Zloty'),
    'SGD' => t('Singapore Dollar'),
    'SEK' => t('Swedish Krona'),
    'CHF' => t('Swiss Franc'),
    'TWD' => t('Taiwan New Dollar'),
    'THB' => t('Thai Baht'),
    'TRY' => t('Turkish Lira'),
  );
}

/**
 * Returns an array of country codes supported by PayPal
 * @see also Appendice C in https://cms.paypal.com/cms_content/US/en_US/files/developer/PP_WebsitePaymentsStandard_IntegrationGuide.pdf
 */
function pinapi_paypal_country_codes() {
  return array(
    'US'	=> t('United States'),
    'CA'	=> t('Canada'),
    'AU'	=> t('Australia'),
    'AT'	=> t('Austria'),
    'BE'	=> t('Belgium'),
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
  );
}