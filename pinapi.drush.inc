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
  $items['pinapi-collection-import'] = array(
    'description' => 'Imports pins to a collection.',
    'arguments' => array(
      'cid' => 'Collection ID.',
    ),
    'required-arguments' => TRUE,
    'options' => array(
      'input' => 'Input directory containing txt files of pins.',
      'sort' => 'Sort the imported file first to improve importing performance.',
      'filter' => 'Filter out a sub string from codes',
      'test' => 'Set pins to be marked as test pins.',
      'quantity' => 'The quantity of each pin being imported.',
      'status' => 'The status of the pin.',
    ),
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
 * Command Callback: Imports pins to a collection.
 */
function drush_pinapi_collection_import($cid) {
  $collection = pinapi_collection_load($cid);

  if ( $collection !== FALSE ) {
    $input = drush_get_option('input', drush_cwd());
    $filter = drush_get_option('filter', NULL);
    $sort = drush_get_option('sort', TRUE);
    $quantity = drush_get_option('quantity', 1);
    $test = drush_get_option('test', 0);
    $status = drush_get_option('status', 1);

    $files = file_scan_directory($input, '/.*\.txt$/');

    if ( !empty($files) ) {
      drush_print(dt('Found @count files in @input', array('@count' => count($files), '@input' => $input)));
      foreach ( $files as $file ) {
        drush_print($file->uri, 2);
      }

      if ( drush_confirm(dt('Import these files into @name collection?:', array('@name' => $collection->name))) ) {
        $delete_after_import = array();
        foreach ( $files as $file ) {
          // Filter out characters from file.
          if ( !is_null($filter) ) {
            $file->uri_filtered = str_replace($file->name, $file->name . '-filtered', $file->uri);
            drush_shell_exec_interactive("cat %s | sed 's/%s/%s/g' > %s", $file->uri, $filter, '', $file->uri_filtered);

            // Update new source file location.
            $file->name = $file->name . '-filtered';
            $file->uri = $file->uri_filtered;

            // Add this file, to list of files to be deleted after importing.
            $delete_after_import[] = $file->uri_filtered;
          }

          // Sort file to improve import performance.
          if ( $sort ) {
            $file->uri_sorted = str_replace($file->name, $file->name . '-sorted', $file->uri);
            drush_shell_exec('sort %s -o %s', $file->uri, $file->uri_sorted);

            // Update new source file location.
            $file->name = $file->name . '-sorted';
            $file->uri = $file->uri_sorted;

            // Add this file, to list of files to be deleted after importing.
            $delete_after_import[] = $file->uri_sorted;
          }

          $sql  = "LOAD DATA INFILE '" . $file->uri . "' ";
          $sql .= "INTO TABLE {pinapi_pin} ";
          $sql .= "FIELDS TERMINATED BY ',' ";
          $sql .= "ENCLOSED BY '' ";
          $sql .= "ESCAPED BY '' ";
          $sql .= "LINES TERMINATED BY '" . '\n' . "' ";
          $sql .= "(code) ";
          $sql .= "SET ";
          $sql .= "code = TRIM(code), ";
          $sql .= "cid = :cid, ";
          $sql .= "quantity_original = :quantity, ";
          $sql .= "quantity = :quantity, ";
          $sql .= "status = :status";

          $args = array(
            ':cid' => $cid,
            ':quantity' => $quantity,
            ':status' => $status,
          );

          $query = db_query($sql, $args);

          // Delete reformatted files.
          foreach ( $delete_after_import as $uri ) {
            file_unmanaged_delete($uri);
          }

          drush_log(dt('Successfully imported @count pins from @input', array('@count' => $query->rowCount(), '@input' => $file->uri)), 'success');
        }
      }
      else {
        drush_set_error('Aborting.');
      }
    }
    else {
      drush_set_error("Could not find .txt files in folder.");
    }
  }
  else {
    drush_set_error("Could not find collection!");
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
