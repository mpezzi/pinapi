<?php

/**
 * @file
 * Provides integration of Pin API with rules.
 */


/**
 * Implements hook_rules_data_info().
 */
function pinapi_rules_rules_data_info() {
  return array(
    'pinapi_txn' => array(
      'label' => t('Pin API Transaction'),
      'wrap' => TRUE,
      'property info' => _pinapi_txn_properties(),
    ),
  );
}

/**
 * Implements hook_rules_event_info().
 */
function pinapi_rules_rules_event_info() {
  return array(
    'pinapi_event_txn_create' => array(
      'label' => t('After creating transaction'),
      'variables' => array(
        'pinapi_txn' => array(
          'type' => 'pinapi_txn',
          'label' => t('Pin API Transaction'),
        ),
      ),
      'group' => t('Pin API'),
    ),
    'pinapi_event_txn_pre_redeem' => array(
      'label' => t('Before redeeming transaction'),
      'variables' => array(
        'pinapi_txn' => array(
          'type' => 'pinapi_txn',
          'label' => t('Pin API Transaction'),
        ),
      ),
      'group' => t('Pin API'),
    ),
    'pinapi_event_txn_redeem' => array(
      'label' => t('After redeeming transaction'),
      'variables' => array(
        'pinapi_txn' => array(
          'type' => 'pinapi_txn',
          'label' => t('Pin API Transaction'),
        ),
      ),
      'group' => t('Pin API'),
    ),
  );
}
