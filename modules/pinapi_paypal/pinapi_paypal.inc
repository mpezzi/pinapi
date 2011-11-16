<?php

/**
 * @file
 * Provides simple PayPal integration.
 */

/**
 * PINAPI Paypal user's PayPal payment status constants
 * Simplified categories for the above PayPal status constants
 */
define('PINAPI_PAYPAL_INCOMPLETE', 0);  // user has not initiated any PayPal payment
define('PINAPI_PAYPAL_COMPLETE', 1);    // user has completed a PayPal payment
define('PINAPI_PAYPAL_PENDING', 2);     // user has a pending PayPal payment
define('PINAPI_PAYPAL_FAILED', 3);      // user's PayPal transaction returned status such as 'Denied', 'Failed', 'Expired', etc.
define('PINAPI_PAYPAL_MISC', 4);        // user's PayPal status returned unusual statuses 'Created' and 'Canceled_Reversal'

/**
 * PayPal Payment status constants
 *
 * Provides more detail about the type of PinAPI status for a particular transaction
 *
 * @see IPN Reference for all PayPal payment_status constants and their meanings
 * https://cms.paypal.com/us/cgi-bin/?cmd=_render-content&content_ID=developer/e_howto_admin_IPNReference
 */
define('PINAPI_PAYPAL_STATUS_COMPLETED', 100);

define('PINAPI_PAYPAL_STATUS_PENDING', 200);
define('PINAPI_PAYPAL_STATUS_PROCESSED', 201);

define('PINAPI_PAYPAL_STATUS_FAILED', 300);
define('PINAPI_PAYPAL_STATUS_DENIED', 301);
define('PINAPI_PAYPAL_STATUS_EXPIRED', 302);
define('PINAPI_PAYPAL_STATUS_REFUNDED', 303);
define('PINAPI_PAYPAL_STATUS_REVERSED', 304);
define('PINAPI_PAYPAL_STATUS_VOIDED', 305);

define('PINAPI_PAYPAL_STATUS_CREATED', 400);    // status is used only by German ELV payments
define('PINAPI_PAYPAL_STATUS_CANCELED_REVERSAL', 401);

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
 * Responds to POSTed data from PayPal IPN, stores to db if verified
 */
function pinapi_paypal_notify() {
  watchdog('pinapi_paypal', json_encode($_POST));

  // create a request query string using POST vars
  $query = array();
  $query['cmd'] = '_notify-validate';
  $query = array_merge($query, $_POST);
  $data = drupal_query_string_encode($query);

  // send request to PayPal to verify transaction being referenced
  $environment = pinapi_paypal_environment();
  $response = drupal_http_request($environment['action'], array(), 'POST', $data, 3);
  watchdog('pinapi_paypal', t("PayPal verification response received: '@data'"), array('@data' => $response->data));

  // only process this POST if verified by PayPal
  if ( $response->data == 'VERIFIED' ) {
    watchdog('pinapi_paypal', t("Received VERIFIED transaction."));

    // attempt to load the transaction details for this uid.
    // Check if a transaction has been previously inserted into db for this user
    // (ex. from a payment marked PENDING)
    $account = pinapi_paypal_load($_POST['custom']);
    if (!$account) {
      $account->is_new = TRUE;
    }

    // is submitted user id a valid user id?
    $user_exists = (bool) user_load($_POST['custom']);

    // validate this transaction against the account we have on record and
    // this site's pinapi_paypal module configuration details
    $valid_transaction = pinapi_paypal_verify_transaction($_POST['txn_id'], $_POST['business'], $_POST['mc_gross'], $_POST['mc_currency'], $account);

    if ($user_exists && $valid_transaction) {
      // set up transaction details to be recorded/updated
      $account->uid = $_POST['custom'];
      $account->tid = $_POST['txn_id'];
      $account->data = serialize($_POST);

      if ( isset($_POST['payment_status']) ) {
        switch ($_POST['payment_status']) {
          case 'Completed':
            if ( $account->is_new ) {
              // set the to-be-written transaction status to 'Completed'
              $account->status = PINAPI_PAYPAL_COMPLETED;
              $account->status_detail = PINAPI_PAYPAL_STATUS_COMPLETED;
              watchdog('pinapi_paypal', t("New transaction record created. User ID: @uid, data: @data"), array('@uid' => $_POST['custom'], '@data' => serialize($_POST)));
            }
            else {
              // check if this is an update to a previously 'pending', 'failed', etc. transaction
              if ($account->status != PINAPI_PAYPAL_COMPLETED) {
                // log what user's payment status was previously
                switch ($account->status) {
                  case PINAPI_PAYPAL_INCOMPLETE:
                    watchdog('pinapi_paypal', t("Transaction record status was updated from 'Incomplete' to 'Complete' for uid: @uid. Data: @data"), array('@uid' => $account->uid, '@data' => serialize($_POST)));
                    break;

                  case PINAPI_PAYPAL_PENDING:
                    watchdog('pinapi_paypal', t("Transaction record status was updated from 'Pending' to 'Complete' for uid: @uid. Data: @data"), array('@uid' => $account->uid, '@data' => serialize($_POST)));
                    break;

                  case PINAPI_PAYPAL_FAILED:
                    watchdog('pinapi_paypal', t("Transaction record status was updated from 'Failed' to 'Complete' for uid: @uid.  Data: @data"), array('@uid' => $account->uid, '@data' => serialize($_POST)));
                    break;

                  case PINAPI_PAYPAL_PENDING:
                    watchdog('pinapi_paypal', t("Transaction record status was updated from 'Pending' to 'Complete' for uid: @uid. Data: @data"), array('@uid' => $account->uid, '@data' => serialize($_POST)));
                    break;

                  default:
                    watchdog('pinapi_paypal', t("Transaction record status was updated from 'Misc' to 'Complete' for uid: @uid.  Data: @data"), array('@uid' => $account->uid, '@data' => serialize($_POST)));
                    break;
                }
                // set the to-be-written transaction status to 'Completed'
                $account->status = PINAPI_PAYPAL_COMPLETED;
                $account->status_detail = PINAPI_PAYPAL_STATUS_COMPLETED;
              }
              else {
                // a duplicate 'Completed' payment submission was detected
                // @TODO : should the old transaction data in db be updated to this new transaction?
                watchdog('pinapi_paypal', t("Duplicate 'Complete' payment detected. New Transaction ID: @tid, uid: @uid, data: @data"), array('@tid' => $_POST['txn_id'], '@uid' => $account->uid, '@data' => serialize($_POST) ), WATCHDOG_WARNING);
              }
            }
            break;

          case 'Pending':
            // handle successful payments marked as 'PENDING' (ex. e-cheque)
            // log reason for pending status
            $account->status = PINAPI_PAYPAL_PENDING;
            $account->status_detail = PINAPI_PAYPAL_STATUS_PENDING;
            break;

          case 'Processed':
            $account->status = PINAPI_PAYPAL_PENDING;
            $account->status_detail = PINAPI_PAYPAL_STATUS_PROCESSED;
            break;

          case 'Created':
            $account->status = PINAPI_PAYPAL_MISC;
            $account->status_detail = PINAPI_PAYPAL_STATUS_CREATED;
            break;

          case 'Canceled_Reversal':
            $account->status = PINAPI_PAYPAL_MISC;
            $account->status_detail = PINAPI_PAYPAL_STATUS_CANCELED_REVERSAL;
            break;

          case 'Failed':
            $account->status = PINAPI_PAYPAL_FAILED;
            $account->status_detail = PINAPI_PAYPAL_STATUS_FAILED;
            break;

          case 'Denied':
            $account->status = PINAPI_PAYPAL_FAILED;
            $account->status_detail = PINAPI_PAYPAL_STATUS_DENIED;
            break;

          case 'Expired':
            $account->status = PINAPI_PAYPAL_FAILED;
            $account->status_detail = PINAPI_PAYPAL_STATUS_EXPIRED;
            break;

          case 'Refunded':
            $account->status = PINAPI_PAYPAL_FAILED;
            $account->status_detail = PINAPI_PAYPAL_STATUS_REFUNDED;
            break;

          case 'Reversed':
            $account->status = PINAPI_PAYPAL_FAILED;
            $account->status_detail = PINAPI_PAYPAL_STATUS_REVERSED;
            break;

          case 'Voided':
            $account->status = PINAPI_PAYPAL_FAILED;
            $account->status_detail = PINAPI_PAYPAL_STATUS_VOIDED;
            break;

          default:
            // handle any other cases where payment status was none of the above
            watchdog('pinapi_paypal', t("PayPal payment cannot be processed because of status '@status': '@data'"), array('@data' => serialize($_POST), '@status' => $_POST['status']),  WATCHDOG_WARNING);
            break;
        }
        // save the data only if a status update was set
        // and log the status to be written to the db
        if ( isset($account->status) ) {
          // log the status update
          $status_name = pinapi_paypal_status_name($account->status_detail);
          if ($account->status !== 'COMPLETED') {
            // identify in the log special PayPal payment statuses with extra variables: 'Pending' and 'Reversed'
            if ($account->status == PINAPI_PAYPAL_PENDING) {
              watchdog('pinapi_paypal', t("PayPal payment marked as PENDING because of '@reason': '@data'"), array('@data' => serialize($_POST), '@reason' => $_POST['pending_reason']),  WATCHDOG_WARNING);
            }
            elseif ($account->status == FAILED && $account->status_detail == PINAPI_PAYPAL_STATUS_REVERSED) {
              watchdog('pinapi_paypal', t("PayPal payment marked as FAILED because PayPal returned status of '@status' for reason of '@reasoncode': @data"), array('@data' => serialize($_POST), '@status' => $_POST['payment_status'], '@reasoncode' => $_POST['reason_code']),  WATCHDOG_WARNING);
            }
            else {
              watchdog('pinapi_paypal', t("PayPal payment marked as @statusname because PayPal returned status of '@status': @data"), array('@data' => serialize($_POST), '@statusname' => $status_name, '@status' => $_POST['payment_status']),  WATCHDOG_WARNING);
            }
          }
          pinapi_paypal_save($account);
        }
      }
    }
    elseif (!$user_exists) {
      watchdog('pinapi_paypal', t("User did not match any on database records. Submitted transaction data: '@data'"), array('@data' => serialize($_POST)),  WATCHDOG_WARNING);
    }
    else {
      // transaction was verified with paypal, but was not validated
      // according to this site's pinapi_paypal and db setup
      watchdog('pinapi_paypal', t("PayPal transaction's details did not match db records or site configuration. User Exists? @user_exists. Submitted transaction data: '@data'"), array('@data' => serialize($_POST), '@user_exists' => $user_exists == 1 ? "TRUE" : "FALSE"),  WATCHDOG_WARNING);
    }
    print '';
  }
  else {
    watchdog('pinapi_paypal', t("Received INVALID transaction..."), array(), WATCHDOG );
    // log this POST with a warning
    watchdog('pinapi_paypal', t("Flagged suspicious PayPal submission: '@data'"), array('@data' => serialize($_POST)), WATCHDOG_ALERT);
  }
}

/**
 * Returns the status category name for a particular detailed status
 *
 * @param
 *   The status code
 * @return
 *   The status category name the supplied status code falls under
 *   or FALSE in case of an unrecognized status
 */
function pinapi_paypal_status_name($s) {

  if ($s >= 400 && $s < 500) {
    return "MISC";
  }
  elseif ($s >= 300) {
    return "FAILED";
  }
  elseif ($s >= 200) {
    return "PENDING";
  }
  elseif ($s >= 100) {
    return "COMPLETED";
  }
  elseif ($s == 0) {
    return "INCOMPLETE";
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