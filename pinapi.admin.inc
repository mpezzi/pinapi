<?php

/**
 * @file
 * Administration page callbacks for the pinapi.module
 */

/**
 * Menu Callback: Display Pin API Administration.
 */
function pinapi_admin_overview() {
  return pinapi_view();
}

/**
 * Displays the pin api administration page.
 */
function pinapi_view() {
  $result = db_query("SELECT * FROM {pinapi_collection} ORDER BY name");

  $output = '<h3>' . t('Collection overview') . '</h3>';

  $header = array(
    t('Name'),
    array('data' => t('Operations'), 'colspan' => '3'),
  );

  $rows = array();
  foreach ( $result as $collection ) {
    $rows[] = array(
      $collection->name,
      l(t('update collection'), "admin/structure/pin/collection/$collection->cid/update"),
      l(t('edit collection'), "admin/structure/pin/collection/$collection->cid/edit"),
      l(t('delete collection'), "admin/structure/pin/collection/$collection->cid/delete"),
    );
  }

  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('No collections available.')));

  return $output;
}

/**
 * Form constructor to add/edit/delete collections.
 */
function pinapi_form_collection($form, &$form_state, $collection = NULL) {
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => isset($collection->name) ? $collection->name : '',
    '#maxlength' => 255,
    '#required' => TRUE,
  );
  $form['machine_name'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($collection->machine_name) ? $collection->machine_name : '',
    '#maxlength' => 255,
    '#machine_name' => array(
      'exists' => 'pinapi_collection_load_machine_name',
    ),
  );
  $form['old_machine_name'] = array(
    '#type' => 'value',
    '#value' => isset($collection->machine_name) ? $collection->machine_name : '',
  );
  $form['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#default_value' => isset($collection->description) ? $collection->description : '',
  );
  $form['entity_type'] = array(
    '#type' => 'textfield',
    '#title' => t('Entity Type'),
    '#default_value' => isset($collection->entity_type) ? $collection->entity_type : '',
    '#maxlength' => 255,
  );
  $form['entity_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Entity ID'),
    '#default_value' => isset($collection->entity_id) ? $collection->entity_id : '',
    '#maxlength' => 128,
  );
  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Published'),
    '#default_value' => isset($collection->status) ? $collection->status : FALSE,
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  if ( isset($collection->cid) ) {
    $form['actions']['delete'] = array('#type' => 'submit', '#value' => t('Delete'));
    $form['cid'] = array('#type' => 'hidden', '#value' => $collection->cid);
  }

  return $form;
}

/**
 * Form validation handler for pinapi_form_collection().
 */
function pinapi_form_collection_validate($form, &$form_state) {
  // @TODO: Check for unique name.
}

/**
 * Form submit handler form pinapi_form_collection().
 */
function pinapi_form_collection_submit($form, &$form_state) {
  if ( $form_state['values']['op'] == t('Save') ) {
    pinapi_collection_save((object) $form_state['values']);

    if ( isset($form_values['cid']) ) {
      drupal_set_message(t('Updated collection %name.', array('%name' => $form_state['values']['name'])));
    }
    else {
      watchdog('pinapi', 'Collection %name has been added.', array('%name' => $form_state['values']['name']));
      drupal_set_message(t('Created new collection %name.', array('%name' => $form_state['values']['name'])));
    }

    $form_state['redirect'] = 'admin/structure/pin';
  }
  else if ( $form_state['values']['op'] == t('Delete') ) {
    pinapi_collection_delete($form_state['values']['cid']);
    watchdog('pinapi', 'Collection %name has been deleted.', array('%name' => $form_state['values']['name']));
    drupal_set_message(t('The collection %name has been deleted.', array('%name' => $form_state['values']['name'])));
    $form_state['redirect'] = 'admin/structure/pin';
  }
}

/**
 * Form constructor to delete collections().
 */
function pinapi_form_collection_delete($form, &$form_state, $collection) {
  return confirm_form(
    array(
      'collection' => array(
        '#type' => 'value',
        '#value' => $collection,
      ),
    ),
    t('Are you sure you want to delete this collection %name?', array('%name' => $collection->name)),
    'admin/structure/pin',
    t('This action cannot be undone.'),
    t('Delete collection'),
    t('Cancel')
  );
}

/**
 * Form submit handler for pinapi_form_collection_delete().
 */
function pinapi_form_collection_delete_submit($form, &$form_state) {
  pinapi_collection_delete($form_state['values']['collection']->cid);
  $form_state['redirect'] = 'admin/structure/pin';
}

/**
 * Form constructor to add/edit/delete transactions.
 */
function pinapi_form_transaction($form, &$form_state, $txn = NULL) {
  $form['uid'] = array(
    '#type' => 'textfield',
    '#title' => t('User'),
    '#default_value' => isset($txn->uid) ? $txn->uid : NULL,
  );
  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Published'),
    '#default_value' => isset($txn->status) ? $txn->status : FALSE,
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  if ( isset($txn->txn_id) ) {
    $form['actions']['delete'] = array('#type' => 'submit', '#value' => t('Delete'));
    $form['txn_id'] = array('#type' => 'hidden', '#value' => $txn->txn_id);
  }

  return $form;
}

/**
 * Form validation handler for pinapi_form_transaction().
 */
function pinapi_form_transaction_validate($form, &$form_state) {
  // @TODO: Check for unique name.
}

/**
 * Form submit handler form pinapi_form_transaction().
 */
function pinapi_form_transaction_submit($form, &$form_state) {
  if ( $form_state['values']['op'] == t('Save') ) {
    $txn = (object) $form_state['values'];
    pinapi_txn_save($txn);

    if ( isset($form_values['txn_id']) ) {
      drupal_set_message(t('Updated transaction.'));
    }
    else {
      watchdog('pinapi', 'Transaction has been created.');
      drupal_set_message(t('Created new transaction.'));
    }

    $form_state['redirect'] = 'admin/structure/pin';
  }
  else if ( $form_state['values']['op'] == t('Delete') ) {
    pinapi_txn_delete($form_state['values']['txn_id']);
    watchdog('pinapi', 'Transaction has been deleted.');
    drupal_set_message(t('The transaction has been deleted.'));
    $form_state['redirect'] = 'admin/structure/pin';
  }
}
