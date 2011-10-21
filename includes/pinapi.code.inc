<?php 

/**
 * @file
 * Provide Pin API Code functionality.
 */

/**
 * Redeem a code.
 *
 * @param $code
 *   A code.
 * @param $pool_id
 *   A pool id.
 * @param $prize
 *   A prize object.
 * @param $account
 *   A user object.
 * @return
 *   Boolean whether code was successfully redeemed.
 */
function pinapi_code_redeem($code, $pool_id, $prize, $account) {
  // Ensure this code can be used.
  if ( !pinapi_code_verify($code, $pool_id, $account) ) {
    return FALSE;
  }

  module_invoke_all('pinapi_code_pre_redeem', $code, $pool_id, $prize, $account);
  
  db_query("UPDATE {pinapi_code} SET quantity = quantity - 1 WHERE code = '%s' AND pool_id = %d AND quantity > 0", $code, $pool_id);
  db_query("UPDATE {pinapi_code} SET status = 0 WHERE code = '%s' AND pool_id = %d AND quantity = 0", $code, $pool_id);
  db_query("INSERT INTO {pinapi_code_redeemed} (code, pool_id, pid, uid, redeem_source, redeemed) VALUES ('%s', %d, %d, %d, '%s', %d)", $code, $pool_id, $prize['pid'], $account->uid, ip_address(), time());
  
  // Get id of the last redeemed prize.
  $prize['crid'] = db_last_insert_id('pinapi_code_redeemed', 'crid');
  
  module_invoke_all('pinapi_code_redeem', $code, $pool_id, $prize, $account);

  return TRUE;
}

/**
 * Verify a code.
 *
 * @param $code
 *   A code.
 * @param $pool_id
 *   A pool id.
 * @param $account
 *   A user object.
 * @param $reset
 *   Reset the internal static cache.
 * @return
 *   Boolean
 */
function pinapi_code_verify($code, $pool_id, $account = NULL, $reset = FALSE) {
  static $codes;
  
  if ( pinapi_pool_is_locked($pool_id) ) {
    if ( !isset($codes[$pool_id][$code]) || $reset ) {
      $result = db_query("SELECT COUNT(status) FROM {pinapi_code} WHERE code = '%s' AND pool_id = %d AND ( quantity > 0 OR quantity = -1 ) AND status = 1 LIMIT 1", $code, $pool_id);
      $codes[$pool_id][$code] = (bool) db_result($result);
    }

    return $codes[$pool_id][$code];
  }
  
  return FALSE;
}

/**
 * Insert code into database.
 */
function pinapi_code_insert($code, $pool_id, $quantity = 1) {
  db_query("INSERT INTO {pinapi_code} (code, pool_id, quantity) VALUES ('". trim($code) ."', ". $pool_id .", ". $quantity .")");
  //db_query("INSERT INTO {pinapi_code} (code, pool_id, quantity) VALUES ('%s', %d, %d)", trim($code), $pool_id, $quantity);
}

/**
 * Clear codes from Pool.
 *
 * @param $pool_id
 *   A pool id.
 * @param $redeemed
 *   Clear redeemed codes.
 */
function pinapi_code_clear($pool_id, $redeemed = FALSE) {
  module_invoke_all('pinapi_code_clear', $pool_id, $redeemed);
  
  // Delete all codes from pool.
  if ( $redeemed ) {
    db_query("DELETE FROM {pinapi_code} WHERE pool_id = %d", $pool_id);
    db_query("DELETE FROM {pinapi_code_redeemed} WHERE pool_id = %d", $pool_id);
  }
  // Delete only non redeemed codes from pool.
  else {
    db_query("DELETE FROM {pinapi_code} WHERE pool_id = %d AND status = 1", $pool_id);
  }
}

/**
 * Clear all codes.
 */
function pinapi_code_clear_all() {
  db_query("TRUNCATE TABLE {pinapi_code}");
}

/**
 * Batch: Import Codes.
 */
function xpinapi_code_import_batch($pool_id, $filepath, $redeemed = FALSE, $limit = 5000) {
  pinapi_code_clear($pool_id, $redeemed);
  
  $total = 0;
  foreach ( pinapi_prizes_select(array('pool_id' => $pool_id, 'status' => 1)) as $prize ) {
    $total = $total + $prize['quantity'];
  }
  
  $operations[] = array('pinapi_code_import_operation', array($pool_id, $total, $filepath, $limit));
  
  return array(
    'operations' => $operations,
    'finished' => 'pinapi_batch_finished',
    'title' => t('Importing codes'),
    'error_message' => t('Importing codes encountered an error.'),
  );
}

/**
 * Batch Operation: Import Codes for a prize.
 */
function xpinapi_code_import_operation($pool_id, $total, $filepath, $limit = 5000, &$context) {
  if ( !isset($context['sandbox']['progress']) ) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current'] = 1;
    $context['sandbox']['max'] = $total;
  }
  
  if ( $limit > $total ) {
    $limit = $total;
  }
  
  $lines = pinapi_code_import_file_read($filepath, $context['sandbox']['progress'], $limit);
  foreach ( $lines as $line ) {
    if ( !empty($line) ) {
      pinapi_code_insert($line, $pool_id);
      $context['sandbox']['progress']++;
    }
  }
  
  if ( $context['sandbox']['progress'] != $context['sandbox']['max'] ) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Read File.
 */
function xpinapi_code_import_file_read($filepath, $offset, $limit = 5000) {
  $file = new SPLFileObject($filepath);
  $file->seek($offset);
  $i = 1;
  $data = array();
  
  if ( $limit == -1 ) {
    while ( !$file->eof() ) {
      $data[] = $file->current();
      $file->next();
      $i++;
    }
  }
  else {
    while ( !$file->eof() && $i <= $limit ) {
      $data[] = $file->current();
      $file->next();
      $i++;
    }
  }
  
  return $data;
}



/**
 * Generate a code.
 */
function pinapi_code_generate($method = 'generic', $options = array()) {
  //return call_user_func('pinapi_code_generate_' . $method, $options);
  return pinapi_code_generate_generic($options);
}

/**
 * Generation Method: Generic.
 */
function pinapi_code_generate_generic($options) {
  return strtoupper(substr(sha1(time() . $options['salt'] . rand()), 0, $options['length']));
}

/**
 * Generate an insert sql statement.
 */
function pinapi_code_generate_insert_query($method, $options = array(), $amount) {
  $sql = "INSERT INTO {pinapi_code} (code, pool_id) VALUES ";

  for ( $i = 0; $i < $amount; $i++ ) {
    $sql .= ( $i != 0 ) ? ', ' : '';
    $sql .= "('" . pinapi_code_generate($method, $options) . "', 1)";
  }
  
  return $sql;
}


/**
 * Return an array of code generators.
 */
function xpinapi_code_generators() {
  $generators = array();
  foreach ( module_invoke_all('pinapi_code_generator') as $key => $generator ) {
    $generators[$key] = $generator['name'];
  }
  return $generators;
}

/**
 * Batch: Generate Codes.
 */
function xpinapi_code_generate_batch($pool_id, $generator = 'pinapi_code_generate_md5_timestamp', $redeemed = FALSE, $limit = 5000) {
  pinapi_code_clear($pool_id, $redeemed);
  
  foreach ( pinapi_prizes_select(array('pool_id' => $pool_id)) as $prize ) {
    $operations[] = array('pinapi_code_generate_operation', array($prize, $generator, $limit));
  }
  
  return array(
    'operations' => $operations,
    'finished' => 'pinapi_batch_finished',
    'title' => t('Generating codes'),
    'error_message' => t('Generating codes encountered an error.'),
  );
}

/**
 * Batch Operation: Generate Codes for a prize.
 */
function xpinapi_code_generate_operation($prize, $generator, $limit = 5000, &$context) {
  if ( !isset($context['sandbox']['progress']) ) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current'] = 1;
    $context['sandbox']['max'] = $prize['quantity'];
  }
  
  for ( $i = 0; $i < $limit; $i++ ) {
    if ( $context['sandbox']['progress'] < $context['sandbox']['max'] ) {
      pinapi_code_insert($generator($prize, $context['sandbox']['progress']), $prize['pool_id']);
      $context['sandbox']['progress']++;
    }
  }
  
  if ( $context['sandbox']['progress'] != $context['sandbox']['max'] ) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Invoke a code generator.
 */
function xpinapi_code_generator_invoke($pool_id, $generator) {
  $generators = module_invoke_all('pinapi_code_generator');
  
  // Allow modules to alter generators.
  drupal_alter('pinapi_code_generator', $generators);
  
  if ( isset($generators[$generator]) && function_exists($generators[$generator]['callback']) ) {
    batch_set(pinapi_code_generate_batch($pool_id, $generators[$generator]['callback']));
  }
}

/**
 * Invoke code import.
 */
function pinapi_code_import_invoke($pool_id, $filepath) {
  if ( file_exists($filepath) && is_readable($filepath) ) {
    batch_set(pinapi_code_import_batch($pool_id, $filepath));
  }
}

/**
 * Generate an MD5 hash using prize information.
 */
function xpinapi_code_generate_md5($prize, $i) {
  return substr(md5($prize['pid'] . '00000000' . $i), 0, 10);
}

/**
 * Generate an MD5 hash using prize information and timestamp.
 */
function xpinapi_code_generate_md5_timestamp($prize, $i) {
  return substr(md5($prize['pid'] . '00000000' . $i . '00000000' . time()), 0, 10);
}

/**
 * Code Redeem Form.
 */
function pinapi_code_redeem_form($form_id, $pool_id) {
  global $user;
  
  $form = array();
  
  if ( user_access('redeem codes') && pinapi_pool_is_locked($pool_id) ) {
    
    // Load full user object.
    $user = user_load($user->uid);
    
    $prizes = pinapi_pool_prizes($pool_id, $user);
    
    if ( count($prizes) ) {
      if ( user_access('debug pin api') && variable_get('pinapi_debug', FALSE) ) {
        pinapi_code_redeem_debug_form($form, $pool_id, $user);
      }

      $form['uid'] = array(
        '#type' => 'hidden',
        '#value' => $GLOBALS['user']->uid,
      );

      $form['pool_id'] = array(
        '#type' => 'hidden',
        '#value' => $pool_id,
      );

      $form['code'] = array(
        '#type' => 'textfield',
        '#title' => t('Code'),
        '#size' => 20,
        '#maxlength' => 255,
        '#required' => TRUE,
      );

      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
      );
    }
    else {
      $form['closed'] = array(
        '#value' => t('Closed.'),
      );
    }
  }
  else {
    $form['closed'] = array(
      '#value' => t('Closed.'),
    );
  }
  
  return $form;
}

/**
 * Code Redeem Form: Validate Callback.
 */
function pinapi_code_redeem_form_validate($form, &$form_state) {
  if ( !empty($form_state['values']['code']) && !pinapi_code_verify($form_state['values']['code'], $form_state['values']['pool_id']) ) {
    form_set_error('code', t("The pin you have entered isn't redeemable or has already been redeemed."));
  }
}

/**
 * Code Redeem Form: Submit Callback.
 */
function pinapi_code_redeem_form_submit($form, &$form_state) {
  $account = user_load($form_state['values']['uid']);
  pinapi_redeem($form_state['values']['code'], $form_state['values']['pool_id'], $account);
}

/**
 * Code Redeem Debug Form.
 */
function pinapi_code_redeem_debug_form(&$form, $pool_id, $account = NULL) {
  $availability = array();
  $prizes = pinapi_pool_prizes($pool_id, $account);
  foreach ( $prizes as $prize ) {
    $availability[] = $prize['name'] . ' (' . $prize['quantity'] . ' left)';
  }
  
  $codes = array();
  $result = db_query("SELECT * FROM {pinapi_code} WHERE status = 1 AND ( quantity > 0 OR quantity = -1 ) AND pool_id = %d LIMIT 30", $pool_id);
  while ( $code = db_fetch_array($result) ) {
    $codes[] = $code['code'] . ' x' . $code['quantity'];
  }

  $form['debug'] = array(
    '#type' => 'fieldset',
    '#title' => t('Debug'),
    '#collapsed' => FALSE,
    '#collapsible' => TRUE,
  );

  $form['debug']['prizes'] = array(
    '#type' => 'item',
    '#title' => t('Prizes'),
    '#value' => theme('item_list', $availability),
  );

  $form['debug']['pins'] = array(
    '#type' => 'item',
    '#title' => t('Pins'),
    //'#value' => $codes_count,
    '#description' => implode(', ', $codes),
  );
}