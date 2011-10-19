<?php 

/**
 * @file
 * Provides custom forms and page functionality for Pin API Nodes.
 */

/**
 * Pool Prize Form.
 */
function pinapi_node_pool_prize_form($form_id, $node) {
  $form = array('#tree' => TRUE);
  $locked = pinapi_node_pool_lock_form($form, $node);
  
  $i = 1;
  foreach ( pinapi_prizes_select(array('pool_id' => $node->pinapi['pool']['pool_id'])) as $prize ) {
    $form['pid'][$prize['pid']] = array(
      '#type' => 'hidden',
      '#value' => $prize['pid'],
    );
    
    $form['content_type'][$prize['pid']] = array(
      '#value' => $prize['content_type'],
    );
    
    $form['status'][$prize['pid']] = array(
      '#type' => 'checkbox',
      '#default_value' => $prize['status'],
      '#disabled' => $locked,
    );
    
    $form['name'][$prize['pid']] = array(
      '#value' => $prize['name'],
    );
    
    $form['quantity'][$prize['pid']] = array(
      '#type' => 'textfield',
      '#default_value' => $prize['quantity'],
      '#size' => 7,
      '#disabled' => $locked,
      '#attributes' => array('tabindex' => $i),
    );
    
    // Invoke _pinapi_prize_operations_type hook and get admin links.
    $operations = pinapi_prize_operations($prize);
    $form['operations'][$prize['pid']] = array(
      '#value' => implode(' ', $operations),
    );
    
    $i++;
  }
  
  // Locked pools should be prevented from saving.
  if ( !$locked ) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#weight' => 101,
    );
  }
  
  return $form;
}

/**
 * Pool Prize Form: Validate Handler.
 */
function pinapi_node_pool_prize_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  
  foreach ( $values['pid'] as $pid ) {
    if ( !empty($values['quantity'][$pid]) && !is_numeric($values['quantity'][$pid]) ) {
      form_set_error('quantity]['. $pid, t('Quantity value must be a number.'));
    }
  }
}

/**
 * Pool Prize Form: Submit Handler.
 */
function pinapi_node_pool_prize_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  
  $prizes = array();
  foreach ( $values['pid'] as $pid ) {
    $prizes[] = array(
      'pid' => $pid,
      'quantity' => $values['quantity'][$pid],
      'status' => $values['status'][$pid],
    );
  }
  
  pinapi_prizes_set($prizes);
  
  drupal_set_message( t('Prizes were saved successfully.') );
}

/**
 * Pool Lock / Unlock Form.
 */
function pinapi_node_pool_lock_form(&$form, $node) {
  $locked = $node->pinapi['pool']['locked'];
  
  $form['pool_id'] = array('#type' => 'hidden', '#value' => $node->pinapi['pool']['pool_id']);
  $form['locked'] = array('#type' => 'hidden', '#value' => $locked);
  
  $form['lock'] = array(
    '#type' => 'submit',
    '#description' => t('Changes to prize quantities can only be done when a pool is unlocked.'),
    '#submit' => array('pinapi_node_pool_lock_form_submit'),
    '#value' => $locked ? t('Unlock') : t('Lock'),
    '#weight' => 100,
  );
  
  if ( $locked ) {
    $form['locked_message'] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="messages status">',
      '#suffix' => '</div>',
      '#value' => t('The Pool is currently locked, please unlock it to change its settings.'),
      '#weight' => -100,
    );
  }
  
  return $locked;
}


/**
 * Pool Lock / Unlock Form: Submit Handler.
 */
function pinapi_node_pool_lock_form_submit($form, &$form_state) {
  if ( $form_state['values']['locked'] ) {
    pinapi_pool_unlock($form_state['values']['pool_id']);
  }
  else {
    pinapi_pool_lock($form_state['values']['pool_id']);
  }
}

/**
 * Pool Code Form.
 */
function pinapi_node_pool_code_form($form_id, $node) {
  $form = array('#tree' => TRUE);
  $locked = pinapi_node_pool_lock_form($form, $node);
  
  $form['pool_id'] = array(
    '#type' => 'hidden',
    '#value' => $node->pinapi['pool']['pool_id'],
  );
  
  $available_prizes = 0;
  $prizes = pinapi_pool_prizes($node->pinapi['pool']['pool_id']);
  foreach ( $prizes as $prize ) {
    $available_prizes = $available_prizes + $prize['quantity'];
  }
  
  $codes = db_result(db_query("SELECT SUM(quantity) FROM {pinapi_code} WHERE quantity > 0 AND status = 1 AND pool_id = %d", $node->pinapi['pool']['pool_id']));
  
  $form['codes'] = array(
    '#type' => 'markup',
    '#prefix' => '<div class="messages status">',
    '#suffix' => '</div>',
    '#value' => '<strong>' . t('Prizes') . ' / ' . t('Codes') . ':</strong> ' . $available_prizes . ' / ' . ( !empty($codes) ? $codes : 0 ),
  );
  
  // Generate.
  $form['generate'] = array(
    '#type' => 'fieldset',
    '#title' => t('Generate'),
    '#description' => t('Code generating method will generate codes based on prize information.'),
    '#collapsed' => FALSE,
    '#collapsible' => TRUE,
  );
  
  $form['generate']['generator'] = array(
    '#type' => 'select',
    '#title' => t('Method'),
    '#options' => pinapi_code_generators(),
    '#disabled' => $locked,
  );
  
  $form['generate']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Generate'),
    '#submit' => array('pinapi_node_pool_code_generate_form_submit'),
    '#disabled' => $locked,
  );
  
  $form['generate']['clear'] = array(
    '#type' => 'submit',
    '#value' => t('Clear'),
    '#submit' => array('pinapi_node_pool_code_clear_form_submit'),
    '#disabled' => $locked,
  );
  
  $form['generate']['clearall'] = array(
    '#type' => 'submit',
    '#value' => t('Clear and Clear Redeemed'),
    '#submit' => array('pinapi_node_pool_code_clearall_form_submit'),
    '#disabled' => $locked,
  );
  
  // Import.
  $form['#attributes'] = array(
    'enctype' => 'multipart/form-data',
  );
  
  $form['import'] = array(
    '#type' => 'fieldset',
    '#title' => t('Import'),
    '#description' => t('Import custom codes from a file or by pasting it below in the text area.'),
    '#collapsed' => FALSE,
    '#collapsible' => TRUE,
  );
  
  $form['import']['file'] = array(
    '#type' => 'file',
    '#title' => t('Upload'),
    '#description' => t('Allowed extensions: txt, csv'),
    '#disabled' => $locked,
  );
  
  $form['import']['paste'] = array(
    '#type' => 'textarea',
    '#title' => t('Paste'),
    '#description' => t('List of pin numbers to import (one per line)'),
    '#rows' => 10,
    '#disabled' => $locked,
  );
  
  $form['import']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#validate' => array('pinapi_node_pool_code_import_form_validate'),
    '#submit' => array('pinapi_node_pool_code_import_form_submit'),
    '#disabled' => $locked,
  );
  
  return $form;
}

/**
 * Pool Code Generate Form: Submit Callback.
 */
function pinapi_node_pool_code_generate_form_submit($form, &$form_state) {
  pinapi_code_generator_invoke($form_state['values']['pool_id'], $form_state['values']['generate']['generator']);
  drupal_set_message(t('Pin numbers have been generated'));
}

/**
 * Pool Code Clear Form: Submit Callback.
 */
function pinapi_node_pool_code_clear_form_submit($form, &$form_state) {
  pinapi_code_clear($form_state['values']['pool_id']);
  drupal_set_message(t('Pin numbers have been cleared.'));
}

/**
 * Pool Code Clear All Form: Submit Callback.
 */
function pinapi_node_pool_code_clearall_form_submit($form, &$form_state) {
  pinapi_code_clear($form_state['values']['pool_id'], TRUE);
  drupal_set_message(t('All pin numbers have been cleared.'));
}

/**
 * Pool Code Import Form: Validate Callback.
 */
function pinapi_node_pool_code_import_form_validate($form, &$form_state) {
  if ( empty($form_state['values']['import']['paste']) ) {
    $file = file_save_upload('import', array(
      'file_validate_extensions' => array('txt csv'),
    ));

    // File has passed validation.
    if ( isset($file->filename) ) {
      $form_state['storage']['import'] = $file;
    }
    else {
      form_set_error('import', t('Invalid file, only files with the extensions txt and csv are allowed'));
    }
  }
}

/**
 * Pool Code Import Form: Submit Callback.
 */
function pinapi_node_pool_code_import_form_submit($form, &$form_state) {
  $pins = array();
  $values = $form_state['values'];
  
  // Paste import method.
  if ( !empty($values['import']['paste']) ) {
    $codes = explode("\n", $values['import']['paste']);
    
    foreach ( $codes as $code ) {
      if ( !empty($code) ) {
        pinapi_code_insert($code, $values['pool_id']);
      }
    }
  }
  // File upload method.
  else {
    pinapi_code_import_invoke($values['pool_id'], $form_state['storage']['import']->filepath);
  }
  
  drupal_set_message(t('Pin codes have been imported.'));
}
