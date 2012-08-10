<?php

/**
 * @file
 * Provides views information.
 */


/**
 * Implements hook_views_data().
 */
function pinapi_views_data() {
  $data['pinapi_collection']['table']['group'] = t('Pin Collections');
  $data['pinapi_collection']['table']['base'] = array(
    'field' => 'cid',
    'title' => t('Pin Collections'),
    'help' => t('Pin Collection Data.'),
  );
  $data['pinapi_collection']['cid'] = array(
    'title' => t('Cid'),
    'help' => t('Collection ID'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );
  $data['pinapi_collection']['name'] = array(
    'title' => t('Name'),
    'help' => t('Human friendly name'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['pinapi_collection']['machine_name'] = array(
    'title' => t('Machine Name'),
    'help' => t('Machine Name'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['pinapi_collection']['description'] = array(
    'title' => t('Description'),
    'help' => t('Description'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['pinapi_collection']['entity_type'] = array(
    'title' => t('Entity Type'),
    'help' => t('Entity Type'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['pinapi_collection']['entity_id'] = array(
    'title' => t('Entity ID'),
    'help' => t('Entity ID'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );
  $data['pinapi_collection']['status'] = array(
    'title' => t('Published'),
    'help' => t('Weather or not the collection is published.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
      'output formats' => array(
        'published-notpublished' => array(t('Published'), t('Not published')),
      ),
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Published'),
      'type' => 'yes-no',
      'use equal' => TRUE, // Use status = 1 instead of status <> 0 in WHERE statment
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['pinapi_pin']['table']['group'] = t('Pins');
  $data['pinapi_pin']['table']['base'] = array(
    'field' => 'pid',
    'title' => t('Pins'),
    'help' => t('Pin Data.'),
  );
  $data['pinapi_pin']['pid'] = array(
    'title' => t('Pid'),
    'help' => t('Pin ID'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );
  $data['pinapi_pin']['cid'] = array(
    'title' => t('Cid'),
    'help' => t('Collection ID'),
    'relationship' => array(
      'base' => 'pinapi_collection',
      'field' => 'cid',
      'handler' => 'views_handler_relationship',
      'label' => t('Collection ID'),
    ),
  );
  $data['pinapi_pin']['code'] = array(
    'title' => t('Code'),
    'help' => t('Code'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['pinapi_pin']['quantity'] = array(
    'title' => t('Quantity'),
    'help' => t('Pin Quantity'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );
  $data['pinapi_pin']['status'] = array(
    'title' => t('Published'),
    'help' => t('Weather or not the pin is published.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
      'output formats' => array(
        'published-notpublished' => array(t('Published'), t('Not published')),
      ),
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Published'),
      'type' => 'yes-no',
      'use equal' => TRUE, // Use status = 1 instead of status <> 0 in WHERE statment
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['pinapi_txn']['table']['group'] = t('Pin Transactions');
  $data['pinapi_txn']['table']['base'] = array(
    'field' => 'txn_id',
    'title' => t('Pin Transactions'),
    'help' => t('Pin Transaction Data.'),
  );
  /*
  $data['pinapi_txn']['table']['join'] = array(
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
    'pinapi_pin' => array(
      'left_field' => 'pid',
      'field' => 'pid',
    ),
  );
  */
  $data['pinapi_txn']['txn_id'] = array(
    'title' => t('Transaction ID'),
    'help' => t('Transaction ID'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );
  $data['pinapi_txn']['pid'] = array(
    'title' => t('Pid'),
    'help' => t('Pin ID'),
    'relationship' => array(
      'title' => t('Pin'),
      'help' => t('Relate to the pin used in the transaction.'),
      'base' => 'pinapi_pin',
      'field' => 'pid',
      'handler' => 'views_handler_relationship',
      'label' => t('Pins'),
    ),
  );
  $data['pinapi_txn']['uid'] = array(
    'title' => t('Uid'),
    'help' => t('User ID'),
    'relationship' => array(
      'title' => t('User'),
      'help' => t('Relate transaction to the user who redeemed it.'),
      'handler' => 'views_handler_relationship',
      'base' => 'users',
      'field' => 'uid',
      'label' => t('user'),
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_user_name',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
    'field' => array(
      'handler' => 'views_handler_field_user',
    ),
  );
  $data['pinapi_txn']['timestamp'] = array(
    'title' => t('Timestamp'),
    'help' => t('Timestamp'),
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
  $data['pinapi_txn']['changed'] = array(
    'title' => t('Changed'),
    'help' => t('Changed'),
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
  $data['pinapi_txn']['hostname'] = array(
    'title' => t('Hostname'),
    'help' => t('Hostname'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['pinapi_txn']['status'] = array(
    'title' => t('Published'),
    'help' => t('Weather or not the transaction is published.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
      'output formats' => array(
        'published-notpublished' => array(t('Published'), t('Not published')),
      ),
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Published'),
      'type' => 'yes-no',
      'use equal' => TRUE, // Use status = 1 instead of status <> 0 in WHERE statment
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  $data['pinapi_txn']['edit_txn'] = array(
    'field' => array(
      'title' => t('Edit link'),
      'help' => t('Provide a simple link to edit the transaction.'),
      'handler' => 'pinapi_views_handler_field_txn_link_edit',
    ),
  );
  $data['pinapi_txn']['delete_txn'] = array(
    'field' => array(
      'title' => t('Delete link'),
      'help' => t('Provide a simple link to delete the transaction.'),
      'handler' => 'pinapi_views_handler_field_txn_link_delete',
    ),
  );

  return $data;
}