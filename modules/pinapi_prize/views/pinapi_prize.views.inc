<?php

/**
 * @file
 * Provide views data and handers for pinapi_prize.module
 */

/**
 * Implements hook_views_handlers().
 */
function pinapi_prize_views_handlers() {
  return array(
    'handlers' => array(
      'pinapi_prize_views_handler_field_prize_redeemed' => array(
        'parent' => 'views_handler_field',
        'path' => drupal_get_path('module', 'pinapi_prize') . '/views',
      ),
    ),
  );
}

/**
 * Implements hook_views_data().
 */
function pinapi_prize_views_data() {
  // ----------------------------------------------------------------
  // pinapi_prize table
  $data['pinapi_prize']['table']['group'] = t('Pin API Prize');
  $data['pinapi_prize']['table']['join'] = array(
    'pinapi_code_redeemed' => array(
      'left_table' => 'pinapi_prize_redeemed',
      'left_field' => 'pid',
      'field' => 'pid',
    ),
  );

  // name.
  $data['pinapi_prize']['name'] = array(
    'title' => t('Prize name'),
    'help' => t('The prize machine name.'),
    'field' => array(
      'handler' => 'pinapi_prize_views_handler_field_prize_redeemed',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // quantity
  $data['pinapi_prize']['quantity'] = array(
    'title' => t('Prize Quantity'),
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

  // bias
  $data['pinapi_prize']['bias'] = array(
    'title' => t('Prize Bias'),
    'help' => t('The Prize Bias'),
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

  // starts
  $data['pinapi_prize']['starts'] = array(
    'title' => t('Start date'),
    'help' => t('The date the prize starts.'),
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

  // expires
  $data['pinapi_prize']['expires'] = array(
    'title' => t('Expire date'),
    'help' => t('The date the prize expires.'),
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

  // status.
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

  // ----------------------------------------------------------------
  // pinapi_prize_redeemed table

  $data['pinapi_prize_redeemed']['table']['group'] = t('Pin API Prize Redeemed');
  $data['pinapi_prize_redeemed']['table']['join'] = array(
    'pinapi_code_redeemed' => array(
      'left_field' => 'rid',
      'field' => 'rid',
    ),
  );

  $data['pinapi_prize_redeemed']['pid'] = array(
    'title' => t('Prize ID'),
    'help' => t('The prize ID'),
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

  return $data;
}

/**
 * Implements hook_views_data_alter().
 */
function pinapi_prize_views_data_alter(&$data) {
  $relationships = module_invoke_all('pinapi_prize_relationships');

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
}
