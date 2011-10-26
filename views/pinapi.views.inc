<?php

/**
 * @file
 * Provides views data and handlers for pinapi.module
 */


/**
 * Implements hook_views_data().
 */
function pinapi_views_data() {
  // -------------------------------------------------------------------
  // PinAPI Code Redeemed
  $data['pinapi_code_redeemed']['table']['group'] = t('Pin API Code Redeemed');

  $data['pinapi_code_redeemed']['table']['base'] = array(
    'field' => 'rid',
    'title' => t('Redeemed codes'),
    'help' => t('Codes that have been redeemed.'),
    'weight' => 10,
  );

  // Code Redeemed ID.
  $data['pinapi_code_redeemed']['rid'] = array(
    'title' => t('ID'),
    'help' => t('The code redeemed ID'),
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

  // Redeemed code.
  $data['pinapi_code_redeemed']['code'] = array(
    'title' => t('Redeem code'),
    'help' => t('The code that was used to redeem.'),
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

  // User ID
  $data['pinapi_code_redeemed']['uid'] = array(
    'title' => t('User'),
    'help' => t('The User ID that redeemed the code.'),
    'relationship' => array(
      'base' => 'users',
      'base field' => 'uid',
      'handler' => 'views_handler_relationship',
      'label' => t('User'),
    ),
  );

  // Redeem history.
  $data['pinapi_code_redeemed']['redeem_source'] = array(
    'title' => t('Redeem source'),
    'help' => t('The ip address that was used to redeem.'),
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

  // Redeemed time.
  $data['pinapi_code_redeemed']['redeemed'] = array(
    'title' => t('Redeemed date'),
    'help' => t('The date the code was redeemed.'),
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
