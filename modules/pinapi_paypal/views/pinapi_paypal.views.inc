<?php

/**
 * @file
 * Provides views data and handlers for pinapi_paypal.module
 */


/**
 * Implements hook_views_handlers().
 */
function pinapi_paypal_views_handlers() {
  return array(
    'handlers' => array(
      'pinapi_paypal_views_handler_field_payment_status' => array(
        'parent' => 'views_handler_field',
        'path' => drupal_get_path('module', 'pinapi_paypal') . '/views',
      ),
    ),
  );
}

/**
 * Implements hook_views_data().
 */
function pinapi_paypal_views_data() {
  // -------------------------------------------------------------------
  // PinAPI Code Redeemed
  $data['pinapi_paypal_order']['table']['group'] = t('Pin API Paypal Order');
  $data['pinapi_paypal_order']['table']['join'] = array(
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );

  // Code ID.
  $data['pinapi_paypal_order']['txnid'] = array(
    'title' => t('Group name'),
    'help' => t('The group machine name.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // PayPal email.
  $data['pinapi_paypal_order']['payer_email'] = array(
    'title' => t('Group name'),
    'help' => t('The group machine name.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Gross.
  $data['pinapi_paypal_order']['mc_gross'] = array(
    'title' => t('Gross'),
    'help' => t('The gross total of purchase'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Payment status.
  $data['pinapi_paypal_order']['payment_status'] = array(
    'title' => t('Payment Status'),
    'help' => t('The state of the payment.'),
    'field' => array(
      'handler' => 'pinapi_paypal_views_handler_field_payment_status',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter',
      'label' => t('Payment Status'),
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Last updated.
  $data['pinapi_paypal_order']['updated'] = array(
    'title' => t('Updated'),
    'help' => t('The date the transaction was last updated.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  return $data;
}
