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
    'pinapi_pin' => array(
      'label' => t('Pin API Pin'),
      'wrap' => TRUE,
      'property info' => array(
        'pid' => array(
          'type' => 'integer',
          'label' => t('Pin ID'),
          'description' => t('The Pin ID'),
          'restriction' => 'input',
          'getter callback' => 'entity_metadata_verbatim_get',
        ),
        'code' => array(
          'type' => 'text',
          'label' => t('Pin Code'),
          'description' => t('The Pin Code'),
          'restriction' => 'input',
          'getter callback' => 'entity_metadata_verbatim_get',
        ),
      ),
    ),
    'pinapi_txn' => array(
      'label' => t('Pin API Transaction'),
      'wrap' => TRUE,
      'property info' => array(
        'txn_id' => array(
          'type' => 'integer',
          'label' => t('Transaction ID'),
          'description' => t('The transaction ID'),
          'restriction' => 'input',
          'getter callback' => 'entity_metadata_verbatim_get',
        ),
        'user' => array(
          'type' => 'user',
          'label' => t('User'),
          'description' => t('The user that created the transaction'),
          'setter callback' => 'entity_metadata_verbatim_set',
        ),
        'timestamp' => array(
          'type' => 'date',
          'label' => t('Transaction date'),
          'description' => t('The transaction date.'),
          'setter callback' => 'entity_metadata_verbatim_set',
          'getter callback' => 'entity_metadata_verbatim_get',
        ),
        'changed' => array(
          'type' => 'date',
          'label' => t('Transaction last changed date.'),
          'description' => t('The transaction last changed date.'),
          'setter callback' => 'entity_metadata_verbatim_set',
          'getter callback' => 'entity_metadata_verbatim_get',
        ),
        'hostname' => array(
          'type' => 'text',
          'label' => t('Transaction Hostname'),
          'description' => t('The hostname used during the transaction.'),
          'restriction' => 'input',
        ),
      ),
    ),
  );
}

/**
 * Implements hook_rules_event_info().
 */
function pinapi_rules_rules_event_info() {
  return array(
    'pinapi_event_txn_create' => array(
      'label' => t('After creating pin transaction'),
      'variables' => array(
        'pinapi_txn' => array(
          'type' => 'pinapi_txn',
          'label' => t('Pin API Transaction'),
        ),
      ),
      'group' => t('Pin API'),
    ),
    'pinapi_event_txn_pre_redeem' => array(
      'label' => t('Before redeeming pin transaction'),
      'variables' => array(
        'pinapi_txn' => array(
          'type' => 'pinapi_txn',
          'label' => t('Pin API Transaction'),
        ),
      ),
      'group' => t('Pin API'),
    ),
    'pinapi_event_txn_redeem' => array(
      'label' => t('After redeeming pin transaction'),
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