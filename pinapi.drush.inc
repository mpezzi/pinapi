<?php

/**
 * @file
 * Provides Drush integration for the Pin API module.
 */


/**
 * Implements hook_drush_command().
 */
function pinapi_drush_command() {
  $items['pinapi-collection-create'] = array(
    'description' => 'Create collection.',
    'arguments' => array(
      'machine_name' => 'Machine name of the collection.',
    ),
    'required-arguments' => TRUE,
    'options' => array(
      'name' => 'The human friendly name',
      'status' => 'The status of the collection',
    ),
  );
  $items['pinapi-collection-calculate'] = array(
    'description' => 'Calculate collection.',
    'arguments' => array(
      'cid' => 'Collection ID.',
    ),
    'required-arguments' => TRUE,
  );
  $items['pinapi-collection-info'] = array(
    'description' => 'Shows collection information.',
    'arguments' => array(
      'cid' => 'Collection ID.',
    ),
    'required-arguments' => TRUE,
  );
  $items['pinapi-collection-reset'] = array(
    'description' => 'Reset collection.',
    'arguments' => array(
      'cid' => 'Collection ID.',
    ),
    'required-arguments' => TRUE,
  );
  $items['pinapi-collection-clear'] = array(
    'description' => 'Clear collection.',
    'arguments' => array(
      'cid' => 'Collection ID.',
    ),
    'required-arguments' => TRUE,
  );
  $items['pinapi-pin-create'] = array(
    'description' => 'Create pin.',
    'arguments' => array(
      'code' => 'Code',
      'cid' => 'Collection ID',
    ),
    'required-arguments' => TRUE,
    'options' => array(
      'quantity' => 'Quantity',
      'status' => 'Status',
    ),
  );
  $items['pinapi-txn-calculate'] = array(
    'description' => 'Calculate transactions.',
    'arguments' => array(
      'cid' => 'Collection ID.',
    ),
    'required-arguments' => TRUE,
    'options' => array(
      'name' => 'A statistic name',
      'uid' => 'A uid',
    ),
  );
  return $items;
}

/**
 * Command Callback: Create Collection.
 */
function drush_pinapi_collection_create($machine_name) {
  $name = drush_get_option('name');
  $status = drush_get_option('status', 1);
  $new_collection = (object) array(
    'machine_name' => $machine_name,
    'name' =>  $name ? $name : $machine_name,
    'status' => $status,
  );
  $result = db_query("SELECT cid FROM {pinapi_collection} WHERE machine_name = :machine_name", array(':machine_name' => $machine_name));
  if ( drush_db_result($result) === FALSE ) {
    if ( !drush_get_context('DRUSH_SIMULATE') ) {
      $new_collection_object = pinapi_collection_save($new_collection);
      if ( $new_collection_object !== FALSE ) {
        drush_log(t('Created @name collection.', array('@name' => $machine_name)), 'success');
        return $collection->cid;
      }
      else {
        drush_set_error("Could not create a new collection with the name " . $machine_name . "!");
      }
    }
  }
  else {
    drush_set_error("There is already a collection with the name " . $machine_name);
  }
}

/**
 * Command Callback: Show collection information.
 */
function drush_pinapi_collection_info($cid) {
  $collection = pinapi_collection_load($cid);

  if ( $collection !== FALSE ) {
    $collection_info = array(
      'Name' => $collection->name,
      'Machine Name' => $collection->machine_name,
      'Status' => $collection->status ? 'active' : 'inactive',
    );
    foreach ( $collection->inventory as $key => $value ) {
      $collection_info[$key] = $value;
    }
    drush_print_table(drush_key_value_to_array_table($collection_info));
  }
  else {
    drush_set_error("Could not find collection!");
  }
}

/**
 * Command Callback: Calculate collection.
 */
function drush_pinapi_collection_calculate($cid) {
  $collection = pinapi_collection_load($cid);

  if ( $collection !== FALSE ) {
    if ( drush_confirm(t("Calculate @name collection?:", array('@name' => $collection->name))) ) {
      pinapi_collection_calculate($cid);
    }
    else {
      drush_set_error("Aborting.");
    }
  }
  else {
    drush_set_error("Could not find collection!");
  }
}

/**
 * Command Callback: Reset collection.
 */
function drush_pinapi_collection_reset($cid) {
  $collection = pinapi_collection_load($cid);

  if ( $collection !== FALSE ) {
    if ( drush_confirm(t("Reset @name collection?:", array('@name' => $collection->name))) ) {
      pinapi_collection_reset($cid);
    }
    else {
      drush_set_error("Aborting.");
    }
  }
  else {
    drush_set_error("Could not find collection!");
  }
}

/**
 * Command Callback: Reset collection.
 */
function drush_pinapi_collection_clear($cid) {
  $collection = pinapi_collection_load($cid);

  if ( $collection !== FALSE ) {
    if ( drush_confirm(t("Clear @name collection?:", array('@name' => $collection->name))) ) {
      pinapi_collection_clear($cid);
    }
    else {
      drush_set_error("Aborting.");
    }
  }
  else {
    drush_set_error("Could not find collection!");
  }
}

/**
 * Command Callback: Create pin.
 */
function drush_pinapi_pin_create($code, $cid) {
  $quantity = drush_get_option('quantity', 1);
  $status = drush_get_option('status', 1);

  $collection = pinapi_collection_load($cid);

  if ( $collection !== FALSE ) {
    $pin = (object) array(
      'code' => $code,
      'cid' => $collection->cid,
      'quantity' => $quantity,
      'status' => $status,
    );
    pinapi_pin_save($pin);
    drush_log(t('Successfully created pin.'), 'success');
  }
  else {
    drush_set_error("Could not find collection!");
  }
}

/**
 * Command Callback: Calculate transactions.
 */
function drush_pinapi_txn_calculate($cid) {
  $name = drush_get_option('name', 'txn_total');
  $uid = drush_get_option('uid');
  if ( !drush_get_context('DRUSH_SIMULATE') ) {
    pinapi_txn_calculate($name, $cid, $uid);
    drush_log(t('Calculated transactions.'), 'success');
  }
}
