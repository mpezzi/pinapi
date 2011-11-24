<?php

/**
 * @file
 * Provide internal functions for pinapi_paypal.module
 */


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
