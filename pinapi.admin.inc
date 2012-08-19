<?php

/**
 * @file
 * Administration page callbacks for the pinapi.module
 */


/**
 * Displays the pin api administration page.
 */
function pinapi_admin_collection_view() {
  $output = '';

  $header = array(
    t('Name'),
    t('Live Pins'),
    t('Test Pins'),
    array('data' => t('Operations'), 'colspan' => '5'),
    t('Active'),
  );

  $rows = array();
  foreach ( pinapi_collection_load_all() as $collection ) {
    $rows[] = array(
      $collection->name,
      number_format(isset($collection->inventory['available']) ? $collection->inventory['available'] : 0) . '/' . number_format(isset($collection->inventory['total']) ? $collection->inventory['total'] : 0),
      number_format(isset($collection->inventory['available_test']) ? $collection->inventory['available_test'] : 0) . '/' . number_format(isset($collection->inventory['total_test']) ? $collection->inventory['total_test'] : 0),
      l(t('calculate'), "admin/pinapi/collection/$collection->cid/calculate"),
      l(t('reset'), "admin/pinapi/collection/$collection->cid/reset"),
      l(t('clear'), "admin/pinapi/collection/$collection->cid/clear"),
      l(t('edit'), "admin/pinapi/collection/$collection->cid/edit"),
      l(t('delete'), "admin/pinapi/collection/$collection->cid/delete"),
      $collection->status ? t('Yes') : t('No'),
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
    '#title' => t('Active'),
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

    $form_state['redirect'] = 'admin/pinapi/collection';
  }
  else if ( $form_state['values']['op'] == t('Delete') ) {
    pinapi_collection_delete($form_state['values']['cid']);
    watchdog('pinapi', 'Collection %name has been deleted.', array('%name' => $form_state['values']['name']));
    drupal_set_message(t('The collection %name has been deleted.', array('%name' => $form_state['values']['name'])));
    $form_state['redirect'] = 'admin/pinapi/collection';
  }
}

/**
 * Form constructor to calculate collections().
 */
function pinapi_form_collection_calculate($form, &$form_state, $collection) {
  return confirm_form(
    array(
      'collection' => array(
        '#type' => 'value',
        '#value' => $collection,
      ),
    ),
    t('Are you sure you want to calculate this collection %name?', array('%name' => $collection->name)),
    'admin/pinapi/collection',
    t('This action cannot be undone.'),
    t('Calculate collection'),
    t('Cancel')
  );
}

/**
 * Form submit handler for pinapi_form_collection_calculate().
 */
function pinapi_form_collection_calculate_submit($form, &$form_state) {
  pinapi_collection_calculate($form_state['values']['collection']->cid);
  $form_state['redirect'] = 'admin/pinapi/collection';
}

/**
 * Form constructor to reset collections().
 */
function pinapi_form_collection_reset($form, &$form_state, $collection) {
  return confirm_form(
    array(
      'collection' => array(
        '#type' => 'value',
        '#value' => $collection,
      ),
    ),
    t('Are you sure you want to reset this collection %name?', array('%name' => $collection->name)),
    'admin/pinapi/collection',
    t('This action cannot be undone.'),
    t('Reset collection'),
    t('Cancel')
  );
}

/**
 * Form submit handler for pinapi_form_collection_calculate().
 */
function pinapi_form_collection_reset_submit($form, &$form_state) {
  pinapi_collection_reset($form_state['values']['collection']->cid);
  $form_state['redirect'] = 'admin/pinapi/collection';
}

/**
 * Form constructor to clear collections().
 */
function pinapi_form_collection_clear($form, &$form_state, $collection) {
  return confirm_form(
    array(
      'collection' => array(
        '#type' => 'value',
        '#value' => $collection,
      ),
    ),
    t('Are you sure you want to clear this collection %name?', array('%name' => $collection->name)),
    'admin/pinapi/collection',
    t('This action cannot be undone.'),
    t('Clear collection'),
    t('Cancel')
  );
}

/**
 * Form submit handler for pinapi_form_collection_calculate().
 */
function pinapi_form_collection_clear_submit($form, &$form_state) {
  pinapi_collection_clear($form_state['values']['collection']->cid);
  $form_state['redirect'] = 'admin/pinapi/collection';
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
    'admin/pinapi/collection',
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
  $form_state['redirect'] = 'admin/pinapi/collection';
}

/**
 * Transaction View.
 */
function pinapi_transaction_view($txn = NULL) {
  return __FUNCTION__;
}

/**
 * Form constructor to add/edit/delete transactions.
 */
function pinapi_form_transaction($form, &$form_state, $txn = NULL) {

  // Determine username for autocomplete textfield.
  if ( isset($txn->uid) ) {
    $user = user_load($txn->uid);
  }

  $form['user'] = array(
    '#type' => 'textfield',
    '#title' => t('User'),
    '#default_value' => isset($user->name) ? $user->name : NULL,
    '#autocomplete_path' => 'user/autocomplete',
    '#maxlength' => 60,
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
    $user = user_load_by_name($form_state['values']['user']);

    $txn = (object) $form_state['values'];
    $txn->uid = $user->uid;

    pinapi_txn_save($txn);

    if ( isset($form_state['values']['txn_id']) ) {
      drupal_set_message(t('Updated transaction.'));
    }
    else {
      watchdog('pinapi', 'Transaction has been created.');
      drupal_set_message(t('Created new transaction.'));
    }

    $form_state['redirect'] = 'admin/pinapi/transaction';
  }
  else if ( $form_state['values']['op'] == t('Delete') ) {
    pinapi_txn_delete($form_state['values']['txn_id']);
    watchdog('pinapi', 'Transaction has been deleted.');
    drupal_set_message(t('The transaction has been deleted.'));
    $form_state['redirect'] = 'admin/pinapi/transaction';
  }
}

/**
 * Form constructor to delete transactions.
 */
function pinapi_form_transaction_delete($form, &$form_state, $txn) {
  return confirm_form(
    array(
      'txn' => array(
        '#type' => 'value',
        '#value' => $txn,
      ),
    ),
    t('Are you sure you want to delete this transaction?'),
    'admin/pinapi/transaction',
    t('This action cannot be undone.'),
    t('Delete transaction'),
    t('Cancel')
  );
}

/**
 * Form submit handler for deleting transactions
 */
function pinapi_form_transaction_delete_submit($form, &$form_state) {
  pinapi_txn_delete($form_state['values']['txn']->txn_id);
  $form_state['redirect'] = 'admin/pinapi/transaction';
}

/**
 * Pin Form.
 */
function pinapi_form_pin($form, &$form_state, $pin = NULL) {
  $collections = array();
  foreach ( pinapi_collection_load_all() as $collection ) {
    $collections[$collection->cid] = $collection->name;
  }

  $form['cid'] = array(
    '#type' => 'select',
    '#title' => t('Collection'),
    '#default_value' => isset($pin->cid) ? $pin->cid : NULL,
    '#required' => TRUE,
    '#options' => $collections,
  );
  $form['code'] = array(
    '#type' => 'textfield',
    '#title' => t('Code'),
    '#default_value' => isset($pin->code) ? $pin->code : '',
    '#required' => TRUE,
  );
  $form['quantity'] = array(
    '#type' => 'textfield',
    '#title' => t('Quantity'),
    '#default_value' => isset($pin->quantity) ? $pin->quantity : 1,
    '#required' => TRUE,
  );
  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Published'),
    '#default_value' => isset($pin->status) ? $pin->status : TRUE,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  if ( isset($pin->pid) ) {
    $form['actions']['delete'] = array('#type' => 'submit', '#value' => t('Delete'));
    $form['pid'] = array('#type' => 'hidden', '#value' => $pin->pid);
  }

  return $form;
}

/**
 * Pin Form Validation.
 */
function pinapi_form_pin_validate($form, &$form_state) {
  // @TODO: Pin validation.
}

/**
 * Pin Form Submit.
 */
function pinapi_form_pin_submit($form, &$form_state) {
  if ( $form_state['values']['op'] == t('Save') ) {
    $pin = (object) $form_state['values'];
    pinapi_pin_save($pin);

    if ( isset($form_values['pid']) ) {
      drupal_set_message(t('Updated pin.'));
    }
    else {
      watchdog('pinapi', 'Pin has been created.');
      drupal_set_message(t('Created pin.'));
    }
  }
  else if ( $form_state['values']['op'] == t('Delete') ){
    pinapi_pin_delete($form_state['values']['pid']);
    watchdog('pinapi', 'Pin has been deleted.');
    drupal_set_message(t('Pin has been deleted.'));
  }

  $form_state['redirect'] = 'admin/pinapi/pin';
}

/**
 * Form constructor to delete collections().
 */
function pinapi_form_pin_delete($form, &$form_state, $pin) {
  return confirm_form(
    array(
      'pin' => array(
        '#type' => 'value',
        '#value' => $pin,
      ),
    ),
    t('Are you sure you want to delete this pin?'),
    'admin/pinapi/pin',
    t('This action cannot be undone.'),
    t('Delete pin'),
    t('Cancel')
  );
}

/**
 * Form submit handler for pinapi_form_collection_delete().
 */
function pinapi_form_pin_delete_submit($form, &$form_state) {
  pinapi_pin_delete($form_state['values']['pin']->cid);
  $form_state['redirect'] = 'admin/pinapi/pin';
}