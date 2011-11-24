<?php

/**
 * @file
 * Provide internal functions for pinapi_paypal.module
 */


/**
 * Load a PayPal Order.
 */
function pinapi_paypal_order_load($oid) {
  return db_fetch_object(db_query("SELECT * FROM {pinapi_paypal_order} WHERE oid = %d", $oid));
}

/**
 * Save a PayPal Order.
 */
function pinapi_paypal_order_save($order) {
  $order->is_new = empty($order->oid);

  if ( $order->is_new ) {
    drupal_write_record('pinapi_paypal_order', $order);
  }
  else {
    drupal_write_record('pinapi_paypal_order', $order, 'oid');
  }
}
