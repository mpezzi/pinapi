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
  $transaction['uid'] = $paypal->uid;
  $transaction['tid'] = $paypal->tid;
  $transaction['timestamp'] = time();

  if ( isset($paypal->is_new) && $paypal->is_new ) {
    drupal_write_record('pinapi_paypal', $paypal);
    drupal_write_record('pinapi_paypal_transactions', $transaction);
  }
  else {
    drupal_write_record('pinapi_paypal', $paypal, 'uid');
    drupal_write_record('pinapi_paypal_transactions', $transaction);
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
 *   the transaction IPN transaction POST data array
 * @param
 *   the PayPal transaction object from the database to verify against
 *
 * @return
 *   TRUE if supplied transaction details match this site's pinapi_paypal
 *   configuration, FALSE otherwise.
 */
function pinapi_paypal_verify_transaction($transaction, &$paypal) {
  $valid_config = FALSE;

  // @TODO: Verify that the following logic conditions are sufficient...
  // Cases/Conditiions for a transaction to be considered VERIFIED
  // 1.  Completely new transaction: new user, new tid, config matches
  // 2.  Existing User Update A:  user exists in db, already has a tid in db, payment config matches, supplied tid matches
  // 3.  Existing User Update B:  user exists in db, already has a tid in db, payment config matches but supplied tid is new (ex. failed payment + new payment)
  // Note:  cases 2 and 3 are really dependent only on matching config
  //        and user's tid's existence in the database

  // Note: In all cases, transaction matching site config is a MUST, so check that first
  $real_email = variable_get('pinapi_paypal_business_email', null);

  // determine what the correct amount is based on the site's current offer
  $all_items = variable_get('pinapi_paypal_items');
  $offer = variable_get('pinapi_paypal_offer', 1);
  switch($offer) {
    case PINAPI_PAYPAL_FREE_TOASTER:
      $items = array_slice($all_items, 0, 1);
      break;
    case PINAPI_PAYPAL_ALTERNATE_OFFER:
      $items = array_slice($all_items, 0, 2);
      break;
  }

  // calculate total cost
  $real_amount = 0;
  foreach ($items as $item) {
    $real_amount += (int)$item['amount'];
  }

  //$real_amount = variable_get('pinapi_paypal_item_amount', null);
  $real_currency = variable_get('pinapi_paypal_currency_code', null);

  // is submitted user id a valid user id in the database?
  $user_exists = (bool) user_load($transaction['custom']);
  $valid_config = ($real_email == $transaction['receiver_email']) && ($real_amount == $transaction['mc_gross']) && ($real_currency == $transaction['mc_currency']);

  if (!$user_exists) {
    watchdog('pinapi_paypal', "Invalid User ID submitted from PayPal IPN, uid: @uid" , array('@uid' => $transaction['custom']), WATCHDOG_ALERT);
    return FALSE;
  }

  if ($valid_config & $user_exists) {
    // handle new users
    if ( isset($paypal->is_new) && $paypal->is_new ) {
      return TRUE;
    }
    // handle existing users
    else {
      if ( isset($paypal->tid) ) {
        // identify what status this transaction from db has
        $prev_status_name = pinapi_paypal_status_name($paypal->status_detail);
        if ($paypal->tid == $transaction['txn_id']) {
          // don't allow duplicate completed IPNs: i.e. matching tid's and statuses are both "Completed"
          if ( $transaction['payment_status'] == 'Completed' && $paypal->status == PINAPI_PAYPAL_COMPLETE ) {
            watchdog('pinapi_paypal', "Duplicate 'Completed' IPN notification detected, Transaction ID: @tid" , array('@tid' => $transaction['txn_id']), WATCHDOG_ALERT);
            return FALSE;
          }
          // log and disallow duplicate IPNs of any other type
          elseif ($transaction['payment_status'] == $prev_status_name) {
            watchdog('pinapi_paypal', "Duplicate '@statusname' IPN notification detected, Transaction ID: @tid" , array('@tid' => $transaction['txn_id'], '@statusname' => $prev_status_name), WATCHDOG_ALERT);
            return FALSE;
          }
          // allow transaction updates: i.e. same tid, different status
          else {
            return TRUE;
          }
        }
        else {
          // otherwise, this transaction's details are valid
          // should cover cases where tid's differ but
          // transaction status changes have occurred
          // ex. first transaction failed, new transaction submitted complete
          return TRUE;
        }
      }
    }
  }
  else {
    // log which transaction detail was invalid
    if ($real_email != $transaction['business']) {
      watchdog('pinapi_paypal', "Invalid business email submitted, business: @tbusiness", array('@tbusiness' => $transaction['business']), WATCHDOG_ALERT);
    }
    if ($real_amount != $transaction['mc_gross']) {
      watchdog('pinapi_paypal', "Invalid transaction amount detected, amount: @tamount", array('@tamount' => $transaction['mc_gross']), WATCHDOG_ALERT);
    }
    if ($real_currency != $transaction['mc_currency']) {
      watchdog('pinapi_paypal', "Invalid currency code detected, code: @tcurrency", array('@tcurrency' => $transaction['mc_currency']), WATCHDOG_ALERT);
    }
    return FALSE;
  }
}

/**
 * Responds to POSTed data from PayPal IPN, stores to db if verified
 */
function pinapi_paypal_notify() {
  watchdog('pinapi_paypal', "PayPal IPN Received: @data", array('@data' => json_encode($_POST)));

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

    // validate this transaction against the account we have on record and
    // this site's pinapi_paypal module configuration details
    $valid_transaction = pinapi_paypal_verify_transaction($_POST, $account);

    if ($valid_transaction) {
      // set up transaction details to be recorded/updated
      $account->uid = $_POST['custom'];
      $account->tid = $_POST['txn_id'];
      $account->data = serialize($_POST);

      if ( isset($_POST['payment_status']) ) {
        switch ($_POST['payment_status']) {
          case 'Completed':
            if ( $account->is_new ) {
              // set the to-be-written transaction status to 'Completed'
              $account->status = PINAPI_PAYPAL_COMPLETE;
              $account->status_detail = PINAPI_PAYPAL_STATUS_COMPLETED;
              watchdog('pinapi_paypal', t("New transaction record created. User ID: @uid, data: @data"), array('@uid' => $_POST['custom'], '@data' => serialize($_POST)), WATCHDOG_WARNING);
            }
            else {
              // check if this is an update to a previously 'pending', 'failed', etc. transaction
              if ($account->status != PINAPI_PAYPAL_COMPLETE) {
                // log what user's payment status was previously
                $prev_status = pinapi_paypal_status_name($account->status_detail);
                watchdog('pinapi_paypal', t("Transaction record status was updated from '@prevstatus' to 'COMPLETE' for uid: @uid. Data: @data"), array('@prevstatus' => $prev_status, '@uid' => $account->uid, '@data' => serialize($_POST)));
                // set the to-be-written transaction status to 'Completed'
                $account->status = PINAPI_PAYPAL_COMPLETE;
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
          if ($account->status !== PINAPI_PAYPAL_COMPLETE) {
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
    print '';
  }
  else {
    watchdog('pinapi_paypal', t("Received INVALID transaction..."), array(), WATCHDOG_ALERT );
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
    return "COMPLETE";
  }
  elseif ($s == 0) {
    return "INCOMPLETE";
  }

  return FALSE;
}

/**
* Returns the status detail code for a supplied PayPal payment_status
*
* @param
*   the payment_status string from a PayPal IPN notification
*
* @return
*   a status detail code
*
* @see also pinapi_paypal_status_detail_name which does the reverse of this function
*/
function pinapi_paypal_status_detail_code($status) {
  switch($status) {
    case 'Completed':
      $code = PINAPI_PAYPAL_STATUS_COMPLETED;
      break;

    case 'Pending':
      $code = PINAPI_PAYPAL_STATUS_PENDING;
      break;

    case 'Processed':
      $code = PINAPI_PAYPAL_STATUS_PROCESSED;
      break;

    case 'Created':
      $code = PINAPI_PAYPAL_STATUS_CREATED;
      break;

    case 'Canceled_Reversal':
      $code = PINAPI_PAYPAL_STATUS_CANCELED_REVERSAL;
      break;

    case 'Failed':
      $code = PINAPI_PAYPAL_STATUS_FAILED;
      break;

    case 'Denied':
      $code = PINAPI_PAYPAL_STATUS_DENIED;
      break;

    case 'Expired':
      $code = PINAPI_PAYPAL_STATUS_EXPIRED;
      break;

    case 'Refunded':
      $code = PINAPI_PAYPAL_STATUS_REFUNDED;
      break;

    case 'Reversed':
      $code = PINAPI_PAYPAL_STATUS_REVERSED;
      break;

    case 'Voided':
      $code = PINAPI_PAYPAL_STATUS_VOIDED;
      break;

    default:
      $code = FALSE;
      break;
  }
  return $code;
}

/**
* Returns the status detail name for a supplied status_detail code
*
* @param
*   a status_detail code
*
* @return
*   a status detail name corresponding to the suppied status_detail code
*
* @see also pinapi_paypal_status_detail_code which does the reverse of this function
*/
function pinapi_paypal_status_detail_name($detail_code) {
  switch($detail_code) {
    case PINAPI_PAYPAL_STATUS_COMPLETED:
      $status = 'Completed';
      break;

    case PINAPI_PAYPAL_STATUS_PENDING:
      $status = 'Pending';
      break;

    case PINAPI_PAYPAL_STATUS_PROCESSED:
      $status = 'Processed';
      break;

    case PINAPI_PAYPAL_STATUS_CREATED:
      $status = 'Created';
      break;

    case PINAPI_PAYPAL_STATUS_CANCELED_REVERSAL:
      $status = 'Canceled_Reversal';
      break;

    case PINAPI_PAYPAL_STATUS_FAILED:
      $status = 'Failed';
      break;

    case PINAPI_PAYPAL_STATUS_DENIED:
      $status = 'Denied';
      break;

    case PINAPI_PAYPAL_STATUS_EXPIRED:
      $status = 'Expired';
      break;

    case PINAPI_PAYPAL_STATUS_REFUNDED:
      $status = 'Refunded';
      break;

    case PINAPI_PAYPAL_STATUS_REVERSED:
      $status = 'Reversed';
      break;

    case PINAPI_PAYPAL_STATUS_VOIDED:
      $status = 'Voided';
      break;

    default:
      $status = FALSE;
      break;
  }
  return $status;
}

/**
 * Simulates handling a PayPal IPN POST.
 *
 * This is a debugging function only.
 */
function pinapi_paypal_verify() {
  // generate a simulated POST using previously POSTed paypal data
  $account = pinapi_paypal_load(1);
  $transaction = json_decode($account->data, TRUE);
  $query = array();
  $query['cmd'] = '_notify-validate';
  $query = array_merge($query, $transaction);
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
    $account = pinapi_paypal_load($transaction['custom']);
    if (!$account) {
      $account->is_new = TRUE;
    }

    // validate this transaction against the account we have on record and
    // this site's pinapi_paypal module configuration details
    $valid_transaction = pinapi_paypal_verify_transaction($transaction, $account);

    if ($valid_transaction) {
      // set up transaction details to be recorded/updated
      $account->uid = $transaction['custom'];
      $account->tid = $transaction['txn_id'];
      $account->data = serialize($transaction);

      if ( isset($transaction['payment_status']) ) {
        switch ($transaction['payment_status']) {
          case 'Completed':
            if ( $account->is_new ) {
              // set the to-be-written transaction status to 'Completed'
              $account->status = PINAPI_PAYPAL_COMPLETE;
              $account->status_detail = PINAPI_PAYPAL_STATUS_COMPLETED;
              watchdog('pinapi_paypal', t("New transaction record created. User ID: @uid, data: @data"), array('@uid' => $transaction['custom'], '@data' => serialize($transaction)));
            }
            else {
              // check if this is an update to a previously 'pending', 'failed', etc. transaction
              if ($account->status != PINAPI_PAYPAL_COMPLETE) {
                // log what user's payment status was previously
                $prev_status = pinapi_paypal_status_name($account->status);
                watchdog('pinapi_paypal', t("Transaction record status was updated from '@prevstatus' to 'Complete' for uid: @uid. Data: @data"), array('@prevstatus' => $prev_status, '@data' => serialize($transaction)));
                // set the to-be-written transaction status to 'Completed'
                $account->status = PINAPI_PAYPAL_COMPLETE;
                $account->status_detail = PINAPI_PAYPAL_STATUS_COMPLETED;
              }
              else {
                // a duplicate 'Completed' payment submission was detected
                // @TODO : should the old transaction data in db be updated to this new transaction?
                watchdog('pinapi_paypal', t("Duplicate 'Complete' payment detected. New Transaction ID: @tid, uid: @uid, data: @data"), array('@tid' => $transaction['txn_id'], '@uid' => $account->uid, '@data' => serialize($transaction) ), WATCHDOG_WARNING);
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
            watchdog('pinapi_paypal', t("PayPal payment cannot be processed because of status '@status': '@data'"), array('@data' => serialize($transaction), '@status' => $transaction['status']),  WATCHDOG_WARNING);
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
              watchdog('pinapi_paypal', t("PayPal payment marked as PENDING because of '@reason': '@data'"), array('@data' => serialize($transaction), '@reason' => $transaction['pending_reason']),  WATCHDOG_WARNING);
            }
            elseif ($account->status == FAILED && $account->status_detail == PINAPI_PAYPAL_STATUS_REVERSED) {
              watchdog('pinapi_paypal', t("PayPal payment marked as FAILED because PayPal returned status of '@status' for reason of '@reasoncode': @data"), array('@data' => serialize($transaction), '@status' => $transaction['payment_status'], '@reasoncode' => $transaction['reason_code']),  WATCHDOG_WARNING);
            }
            else {
              watchdog('pinapi_paypal', t("PayPal payment marked as @statusname because PayPal returned status of '@status': @data"), array('@data' => serialize($transaction), '@statusname' => $status_name, '@status' => $transaction['payment_status']),  WATCHDOG_WARNING);
            }
          }
          pinapi_paypal_save($account);
        }
      }
    }
    print '';
  }
  else {
    watchdog('pinapi_paypal', t("Received INVALID transaction..."), array(), WATCHDOG_ALERT );
    // log this POST with a warning
    watchdog('pinapi_paypal', t("Flagged suspicious PayPal submission: '@data'"), array('@data' => serialize($_POST)), WATCHDOG_ALERT);
  }
}

/**
 * Sets values for items in the system
 */
function pinapi_paypal_set_items() {
  $items = array(
    array(
      'name' => 'Toaster',
      'amount' => '30.00',
      'quantity' => '1',
    ),
    array(
      'name' => 'Shipping',
      'amount' => '20.00',
      'quantity' => '1',
    )
  );
  variable_set('pinapi_paypal_items', $items);
}

/**
 * Generates PayPal payment form elements from an array of items
 *
 * @param
 *   an array of items
 * @param
 *   the PayPal form to which elements need to be added
 * @param
 *   the number of items to return
 *
 * @return
 *   the PayPal form with item elements added
 */
function pinapi_paypal_generate_form_items($items, &$form, $limit = null) {
  $count = $limit || count($items);
  for ($i = 0; $i < count($items); $i++)  {
    $form['item_name_' . ($i+1)] = array(
      '#type' => 'hidden',
      '#value' => $items[$i]['name'],
    );
    $form['amount_' . ($i+1)] = array(
      '#type' => 'hidden',
      '#value' => $items[$i]['amount'],
    );
    $form['quantity_' . ($i+1)] = array(
      '#type' => 'hidden',
      '#value' => $items[$i]['quantity'],
    );
  }
  return $form;
}