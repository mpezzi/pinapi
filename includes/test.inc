<?php

/**
 * @file
 * Provides internal testing functions for Pin API module.
 */

/**
 * Builds a test environment for Pin API.
 */
function pinapi_test_build() {
  // Create test collection.
  $collection = pinapi_test_collection_create();

  // Create 500 test pins for collection.
  pinapi_test_pin_create($collection->cid, 500, TRUE, 'test');

  // Create 500 live pins for collection.
  pinapi_test_pin_create($collection->cid, 500, FALSE, 'live');

  // Calculate collection inventory.
  pinapi_collection_calculate($collection->cid);

  // Select all users (except anonymous and admin)
  $result = db_select('users', 'u')
    ->fields('u', array('uid'))
    ->condition('uid', 0, '>')
    ->execute();

  $accounts = array();
  foreach ( $result as $row ) {
    $accounts[$row->uid] = $row->uid;
  }

  // Select live pins.
  $result = db_select('pinapi_pin', 'p')
    ->fields('p', array('pid', 'code'))
    ->condition('cid', $collection->cid)
    ->condition('test', 0)
    ->execute();

  $codes = array();
  foreach ( $result as $row ) {
    $codes[] = $row->code;
  }

  // Randomly redeem pins for user accounts.
  pinapi_test_txn_redeem($collection, $accounts, $codes);

  drupal_set_message(t('Successfully built test environment.'));
}

/**
 * Creates test a collection.
 */
function pinapi_test_collection_create() {
  $collection = (object) array(
    'name' => 'Test',
    'machine_name' => 'test',
    'status' => 1,
  );

  // Save collection.
  pinapi_collection_save($collection);

  drupal_set_message(t('Successfully created @name collection.', array('@name' => $collection->name)));

  return $collection;
}

/**
 * Creates test pins.
 */
function pinapi_test_pin_create($cid, $number_of_pins = 5, $test = TRUE, $prefix = 'test') {
  for ( $i = 0; $i < $number_of_pins; $i++ ) {
    $pin = (object) array(
      'cid' => $cid,
      'code' => $prefix . $i,
      'quantity_original' => 1,
      'quantity' => 1,
      'test' => $test,
      'status' => 1,
    );
    pinapi_pin_save($pin);
  }

  drupal_set_message(t('Successfully created @number_of_pins pins.', array('@number_of_pins' => $number_of_pins)));
}

/**
 * Creates test transactions.
 */
function pinapi_test_txn_redeem($collection, $accounts, $codes) {
  foreach ( $codes as $code ) {
    // Create a transaction.
    $txn = pinapi_txn($code, $collection->cid, array_rand($accounts));

    // Redeem a transaction.
    pinapi_txn_redeem($txn);
  }
}
