<?php

/**
 * @file
 * Provide views data and handers for pinapi_prize.module
 */


/**
 * Implements hook_views_data().
 */
function pinapi_prize_views_data() {
  // ----------------------------------------------------------------
  // pinapi_prize_redeemed table

  $data['pinapi_prize_redeemed']['table']['group'] = t('Pin API Prize');

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