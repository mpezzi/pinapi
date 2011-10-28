<?php

/**
 * @file
 * Provides views data and handlers for pinapi.module
 */


/**
 * Implements hook_views_handlers().
 */
function pinapi_views_handlers() {
  return array(
    'handlers' => array(
      'pinapi_views_handler_relationship' => array(
        'parent' => 'views_handler_relationship',
        'path' => drupal_get_path('module', 'pinapi') . '/views',
      ),
    ),
  );
}

/**
 * Implements hook_views_data().
 */
function pinapi_views_data() {
  // -------------------------------------------------------------------
  // PinAPI Code Redeemed
  $data['pinapi_code']['table']['group'] = t('Pin API Code');
  $data['pinapi_code']['table']['join'] = array(
    'pinapi_code_redeemed' => array(
      'left_field' => 'cid',
      'field' => 'cid',
    ),
  );

  // Code ID.
  $data['pinapi_code']['cid'] = array(
    'title' => t('Code ID'),
    'help' => t('The ID of the code'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'numeric' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Quantity
  $data['pinapi_code']['quantity'] = array(
    'title' => t('Code Quantity'),
    'help' => t('The Code Quantity'),
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

  // Status.
  $data['pinapi_code']['status'] = array(
    'title' => t('Published'),
    'help' => t('Whether or not the code is published.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Published'),
      'type' => 'yes-no',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // -------------------------------------------------------------------
  // PinAPI Code Redeemed
  $data['pinapi_group']['table']['group'] = t('Pin API Group');
  $data['pinapi_group']['table']['join'] = array(
    'pinapi_code_redeemed' => array(
      'left_field' => 'gid',
      'field' => 'gid',
    ),
  );

  // Group ID.
  $data['pinapi_group']['gid'] = array(
    'title' => t('Group ID'),
    'help' => t('The ID of the group'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument',
      'numeric' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Group name.
  $data['pinapi_group']['name'] = array(
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

  // Group status.
  $data['pinapi_group']['status'] = array(
    'title' => t('Published'),
    'help' => t('Whether or not the group is published.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Published'),
      'type' => 'yes-no',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

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
    'title' => t('Redeem ID'),
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

  // Code ID.
  $data['pinapi_code_redeemed']['cid'] = array(
    'title' => t('Code'),
    'help' => t('The Pin API Code this redeemed code is from.'),
    'relationship' => array(
      'base' => 'pinapi_code',
      'base field' => 'cid',
      'handler' => 'views_handler_relationship',
      'label' => t('Pin API Code'),
    ),
  );

  // Group ID.
  $data['pinapi_code_redeemed']['gid'] = array(
    'title' => t('Group'),
    'help' => t('The Pin API Group this redeemed code is from.'),
    'relationship' => array(
      'base' => 'pinapi_group',
      'base field' => 'gid',
      'handler' => 'views_handler_relationship',
      'label' => t('Pin API Group'),
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

  // Status.
  $data['pinapi_code_redeemed']['status'] = array(
    'title' => t('Published'),
    'help' => t('Whether or not the redeemed code is available.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Published'),
      'type' => 'yes-no',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  return $data;
}

/**
 * Implements hook_views_data_alter().
 */
function pinapi_views_data_alter(&$data) {
  $relationships = module_invoke_all('pinapi_relationships');

  if ( isset($relationships['pinapi_group']) ) {
    foreach ( $relationships['pinapi_group'] as $type => $relationship ) {
      $data['pinapi_group'][$type] = array(
        'title' => $relationship['title'],
        'help' => $relationship['help'],
        'relationship' => array(
          'base' => $relationship['base'],
          'base field' => $relationship['base field'],
          'handler' => $relationship['handler'],
          'label' => $relationship['title'],
        ),
      );
    }
  }
}
