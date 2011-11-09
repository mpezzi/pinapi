<?php 

/**
 * @file
 * Provide Pin API Code functionality.
 */


/**
 * Redeem a code.
 *
 * @param $code
 *   A string.
 * @param $group
 *   A group.
 * @param $account
 *   A user account.
 * @return
 *   TRUE or FALSE depending if the code is valid.
 */
function pinapi_code_redeem($string, $group, $account = NULL) {
  if ( $code = pinapi_code_verify($string, $group, $account, TRUE) ) {

    // Invoke module hook_pinapi_codeapi().
    pinapi_codeapi_invoke('pre redeem', $code, $group, $account);

    // Modules can prevent this code from being redeemable if they choose.
    if ( $code->redeemable ) {

      // Redeem this code in the database, set appropriate quantities.
      db_query("UPDATE {pinapi_code} SET quantity = quantity - 1 WHERE cid = %d AND gid = %d AND quantity > 0", $code->cid, $group->gid);
      db_query("UPDATE {pinapi_code} SET status = 0 WHERE cid = %d AND gid = %d AND quantity = 0", $code->cid, $group->gid);

      // Create a redeem record to keep track of redeemed codes.
      db_query("INSERT INTO {pinapi_code_redeemed} (code, cid, gid, uid, redeem_source, redeemed) VALUES ('%s', %d, %d, %d, '%s', %d)", $string, $code->cid, $group->gid, $account->uid, ip_address(), time());

      // Get id of the last redeemed code.
      $code->rid = db_last_insert_id('pinapi_code_redeemed', 'rid');

      // Insert or update cached redeemed count.
      pinapi_code_redeemed_cache_add_count($group, $account);

      // Update the cached group code count.
      pinapi_group_cache_code_redeem($group);

      // Invoke module hook_pinapi_codeapi().
      pinapi_codeapi_invoke('redeem', $code, $group, $account);

      return TRUE;
    }
  }

  // Code was not verified, therefore cannot be redeemed.
  return FALSE;
}

/**
 * Verify a code.
 *
 * @param $string
 *   A code string.
 * @param $group
 *   A group object.
 * @param $account
 *   A user account.
 * @param $reset
 *   Reset the internal static cache.
 * @return
 *   TRUE or FALSE depending if the code is valid.
 */
function pinapi_code_verify($string, $group, $account = NULL, $reset = FALSE) {
  static $verify;

  if ( $group->status ) {
    if ( !isset($verify[$group->gid][$string]) || $reset ) {
      $sql = "SELECT * FROM {pinapi_code} WHERE code = '%s' AND gid = %d AND status = 1 LIMIT 1";
      $verify[$group->gid][$string] = db_fetch_object(db_query($sql, $string, $group->gid));
    }

    return $verify[$group->gid][$string];
  }

  return FALSE;
}

/**
 * Invoke the code generator.
 *
 * @return
 *   A generator object or FALSE
 */
function pinapi_code_get_generator($type = NULL) {
  if ( is_null($type) ) {
    $type = variable_get('pinapi_code_generator', 'generic');
  }

  // Invoke modules for their code generators and allow them the ability to alter them.
  $generators = module_invoke_all('pinapi_code_generator');
  drupal_alter('pinapi_code_generator', $generators);

  // If generator exists and we have usuable callback, return the generator.
  if ( isset($generators[$type]) && function_exists($generators[$type]['callback']) ) {

    // @TODO: Set default options.
    if ( !isset($generators[$type]['options']) ) {
      $generators[$type]['options'] = array();
    }

    return $generators[$type];
  }

  return FALSE;
}

/**
 * Generate a code.
 *
 * @param $generator
 *   A generator object.
 */
function pinapi_code_generate($generator) {
  return call_user_func($generator['callback'], $generator['options']);
}

/**
 * Generation Method: Generic.
 *
 * @param $options
 *   An array of options to generate the code with.
 * @return
 *   A generated code as string.
 */
function pinapi_code_generate_generic($options) {
  return strtoupper(substr(sha1(time() . $options['salt'] . rand()), 0, $options['length']));
}

/**
 * Return a INSERT sql query to mass generate codes.
 *
 * @param $generator
 *   A generator.
 * @param $amount
 *   The amount of codes to insert.
 * @return
 *   An INSERT sql query as string
 */
function pinapi_code_generate_insert($generator, $amount, $gid = 0) {
  $sql = "INSERT INTO {pinapi_code} (gid, code) VALUES ";

  for ( $i = 0; $i < $amount; $i++ ) {
    $sql .= ( $i != 0 ) ? ', ' : '';
    $sql .= "(" . $gid . ", '" . pinapi_code_generate($generator) . "')";
  }

  return db_query($sql);
}

/**
 * Return a INSERT sql query to mass generate codes.
 *
 * @param $generator
 *   A generator.
 * @param $amount
 *   The amount of codes to insert.
 * @return
 *   An INSERT sql query as string
 */
function pinapi_code_generate_insert_sql($generator, $amount, $gid = 0) {
  $sql = "INSERT INTO `pinapi_code` (gid, code) VALUES ";

  for ( $i = 0; $i < $amount; $i++ ) {
    $sql .= ( $i != 0 ) ? ', ' : '';
    $sql .= "(" . $gid . ", '" . pinapi_code_generate($generator) . "')";
  }

  return $sql . ';';
}

/**
 * Reset codes from a group.
 */
function pinapi_code_reset($gid) {
  // Invoke module hook_pinapi_code_reset().
  module_invoke_all('pinapi_code_reset', $gid);

  // Reset codes.
  db_query("UPDATE {pinapi_code} SET status = 1, quantity = 1 WHERE gid = %d", $gid);

  // Remove all redeem code information.
  db_query("DELETE FROM {pinapi_code_redeemed} WHERE gid = %d", $gid);
  db_query("DELETE FROM {pinapi_code_redeemed_cache} WHERE gid = %d", $gid);

  pinapi_group_cache_clear($gid);
}

/**
 * Clear codes from a group.
 */
function pinapi_code_clear($gid) {
  // Invoke module hook_pinapi_code_clear().
  module_invoke_all('pinapi_code_clear', $gid);

  // Remove both redeemed and non-redeemed codes from group.
  db_query("DELETE FROM {pinapi_code} WHERE gid = %d", $gid);
  db_query("DELETE FROM {pinapi_code_redeemed} WHERE gid = %d", $gid);
  db_query("DELETE FROM {pinapi_code_redeemed_cache} WHERE gid = %d", $gid);

  pinapi_group_cache_clear($gid);
}

/**
 * Clear all codes.
 */
function pinapi_code_clear_all() {
  // Invoke module hook_pinapi_code_clear_all().
  module_invoke_all('pinapi_code_clear_all');

  // Remove code dependent tables.
  db_query("TRUNCATE TABLE {pinapi_code}");
  db_query("TRUNCATE TABLE {pinapi_code_redeemed}");
  db_query("TRUNCATE TABLE {pinapi_code_redeemed_cache}");
  db_query("TRUNCATE TABLE {pinapi_group_cache}");
}

/**
 * Increase the redeem count value for user.
 */
function pinapi_code_redeemed_cache_add_count($group, $account) {
  // Determine wheather we need to insert or update result.
  $exist = db_result(db_query("SELECT COUNT(*) FROM {pinapi_code_redeemed_cache} WHERE uid = %d AND gid = %d", $account->uid, $group->gid));

  if ( !$exist ) {
    db_query("INSERT INTO {pinapi_code_redeemed_cache} (uid, gid, value, timestamp) VALUES (%d, %d, %d, %d)", $account->uid, $group->gid, 1, time());
  }
  else {
    db_query("UPDATE {pinapi_code_redeemed_cache} SET value = value + 1, timestamp = %d WHERE uid = %d AND gid = %d", time(), $account->uid, $group->gid);
  }
}

/**
 * Invoke a hook_pinapi_codeapi operation in all modules.
 */
function pinapi_codeapi_invoke($op, &$code, &$group, &$account) {
  $return = array();

  // By default set a code as redeemable, other module may alter this as needed.
  if ( $op == 'pre redeem' ) {
    $code->redeemable = TRUE;
  }

  foreach ( module_implements('pinapi_codeapi') as $name ) {
    $function = $name . '_pinapi_codeapi';
    $result = $function($op, $code, $group, $account);
    if ( isset($result) && is_array($result) ) {
      $return = array_merge($return, $result);
    }
    else {
      $return[] = $result;
    }
  }

  return $return;
}



















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
function xpinapi_code_redeem($code, $pool_id, $prize, $account) {
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
function xpinapi_code_verify($code, $pool_id, $account = NULL, $reset = FALSE) {
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
function xpinapi_code_insert($code, $pool_id, $quantity = 1) {
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
function xpinapi_code_clear($pool_id, $redeemed = FALSE) {
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
function xpinapi_code_import_invoke($pool_id, $filepath) {
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