<?php 

/**
 * @file
 * Provides Pin API Administrative Forms.
 */


/**
 * Pin API Nodes Administrative Settings Form.
 */
function pinapi_node_settings_form() {
  $form['pool'] = array(
    '#type' => 'fieldset',
    '#title' => t('Pool'),
    '#description' => t('Settings for pools'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
  );
  
  $form['pool']['pinapi_node_pool_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Pool Node Types'),
    '#description' => t('Content types that can have prizes associated to them.'),
    '#options' => node_get_types('names'),
    '#default_value' => variable_get('pinapi_node_pool_types', array()),
  );
  
  $form['prize'] = array(
    '#type' => 'fieldset',
    '#title' => t('Prize'),
    '#description' => t('Settings for prizes'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
  );
  
  $form['prize']['pinapi_node_prize_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Prize Node Types'),
    '#description' => t('Content types that can be redeemed as a prize.'),
    '#options' => node_get_types('names'),
    '#default_value' => variable_get('pinapi_node_prize_types', array()),
  );
  
  return system_settings_form($form);
}