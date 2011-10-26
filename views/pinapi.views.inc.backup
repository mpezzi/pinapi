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
  // PinAPI Pool
  
  $data['pinapi_pool']['table']['group'] = t('Pool');
  
  $data['pinapi_pool']['pool_id'] = array(
    'title' => t('Pool ID'),
    'help' => t('The ID of the pool'),
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
  
  $data['pinapi_pool']['locked'] = array(
    'title' => t('Locked'),
    'help' => t('Whether or not the pool is locked.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Locked'),
      'type' => 'yes-no',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  
  $data['pinapi_pool']['status'] = array(
    'title' => t('Published'),
    'help' => t('Whether or not the pool is published.'),
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
  // PinAPI Prize
  
  $data['pinapi_prize']['table']['group'] = t('Prize');
  
  $data['pinapi_prize']['pid'] = array(
    'title' => t('Pid'),
    'help' => t('The prize ID of the prize'),
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
  
  $data['pinapi_prize']['pool_id'] = array(
    'title' => t('Pool ID'),
    'help' => t('The Pool ID'),
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
    'argument' => array(
      'handler' => 'views_handler_argument',
    ),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'pinapi_pool',
      'field' => 'pool_id',
      'label' => t('Pool'),
    ),
  );
  
  $data['pinapi_prize']['quantity'] = array(
    'title' => t('Quantity'),
    'help' => t('The Prize Quantity'),
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
  
  $data['pinapi_prize']['status'] = array(
    'title' => t('Published'),
    'help' => t('Whether or not the prize is published.'),
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
  
  $data['pinapi_code_redeemed']['table']['group'] = t('Code Redeemed');
  
  $data['pinapi_code_redeemed']['table']['base'] = array(
    'field' => 'crid',
    'title' => t('Redeemed codes'),
    'help' => t('Codes that have been redeemed by users.'),
    'weight' => 10,
  );
  
  $data['pinapi_code_redeemed']['table']['join'] = array(
    'pinapi_prize' => array(
      'left_field' => 'pid',
      'field' => 'pid',
    ),
  );
  
  // Code redeemed ID.
  $data['pinapi_code_redeemed']['crid'] = array(
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
  
  // Pool ID
  $data['pinapi_code_redeemed']['pool_id'] = array(
    'title' => t('Pool'),
    'help' => t('The pool of the prize that was redeemed.'),
    'relationship' => array(
      'base' => 'pinapi_pool',
      'base field' => 'pool_id',
      'handler' => 'views_handler_relationship',
      'label' => t('Pool'),
    ),
  );
  
  // Prize ID
  $data['pinapi_code_redeemed']['pid'] = array(
    'title' => t('Prize'),
    'help' => t('The prize that was redeemed.'),
    'relationship' => array(
      'base' => 'pinapi_prize',
      'base field' => 'pid',
      'handler' => 'views_handler_relationship',
      'label' => t('Prize'),
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
    'title' => t('Redeemed date'), // The item it appears as on the UI,
    'help' => t('The date the code was redeemed.'), // The help that appears on the UI,
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

/**
 * Implements hook_views_data_alter().
 */
function pinapi_views_data_alter(&$data) {
  $relationships = module_invoke_all('pinapi_relationships');
  
  if ( isset($relationships['pinapi_prize']) ) {
    foreach ( $relationships['pinapi_prize'] as $type => $relationship ) {
      $data['pinapi_prize'][$type] = array(
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
  
  if ( isset($relationships['pinapi_pool']) ) {
    foreach ( $relationships['pinapi_pool'] as $type => $relationship ) {
      $data['pinapi_pool'][$type] = array(
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