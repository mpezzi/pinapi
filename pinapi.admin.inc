<?php 

/**
 * @file
 * Configuration forms and helper functions for PinAPI module.
 */


/**
 * Administrative settings for PinAPI.
 */
function pinapi_settings_form($form_state) {
  $form['debug'] = array(
    '#type' => 'fieldset',
    '#title' => t('Debug'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#weight' => -100,
  );
  
  $form['debug']['pinapi_debug'] = array(
    '#type' => 'checkbox',
    '#title' => t('Simple Debug Mode'),
    '#description' => t('Enable debugging messages for users with "debug pin api".'),
    '#default_value' => variable_get('pinapi_debug', FALSE),
  );
  
  return system_settings_form($form);
}

/**
 * Returns Pin API System Report.
 */
function pinapi_report_view() {
  $result = db_query("SELECT * FROM {pinapi_pool} WHERE status = 1");
  
  while ( $pool = db_fetch_array($result) ) {
    $availability = array();
    $prizes = pinapi_pool_prizes($pool['pool_id']);
    foreach ( $prizes as $prize ) {
      $availability[] = $prize['name'] . ' (' . $prize['quantity'] . ' left x' . $prize['bias'] . ')';
    }

    $content['pool_' . $pool['pool_id']] = array(
      '#type' => 'item',
      '#title' => t('Pool @num', array('@num' => $pool['pool_id'])),
      '#value' => theme('item_list', $availability),
      '#weight' => 0,
    );
  }
  
  return drupal_render($content);
}