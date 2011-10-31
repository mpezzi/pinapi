<?php

/**
 * @file
 * Provides Administrative Pages for Pin API UI.
 */


/**
 * Adminstration Page.
 */
function pinapi_ui_view_overview() {
  return __FUNCTION__;
}

/**
 * Group Administration Page.
 */
function pinapi_ui_view_groups() {
  $links = array(
    'add' => l(t('Add Group'), 'admin/build/pinapi/group/add'),
  );
  
  $output = theme('item_list', $links);

  // Fetch all Pin API groups.
  $groups = pinapi_group_get_groups();
  if ( empty($groups) ) {
    return $output . t('No groups exist.');
  }

  $header = array(
    array('data' => t('Name')),
    array('data' => t('Operation')),
  );

  $rows = array();
  foreach ( $groups as $group ) {
    $operations = array(
      'edit' => l(t('Edit'), "admin/build/pinapi/group/$group->gid/edit"),
    );

    $rows[] = array(
      array('class' => 'pinapi-group-name', 'data' => check_plain($group->name)),
      array('class' => 'pinapi-operation', 'data' => implode(' | ', $operations)),
    );
  }

  $output .= theme('table', $header, $rows);
  $output .= theme('pager', NULL, 25);

  return $output;
}

/**
 * Group Edit Form.
 */
function pinapi_ui_group_form(&$form_state, $group = NULL) {
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Machine Name'),
    '#default_value' => isset($group->name) ? $group->name : '',
    '#weight' => -1,
  );

  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Published'),
    '#default_value' => isset($group->status) ? $group->status : NULL,
    '#weight' => 1,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => isset($group->gid) ? t('Update') : t('Add'),
    '#weight' => 20,
  );

  if ( isset($group->gid) ) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#validate' => array('pinapi_ui_group_form_delete_validate'),
      '#submit' => array('pinapi_ui_group_form_delete_submit'),
      '#weight' => 21,
    );
    $form['gid'] = array(
      '#type' => 'hidden',
      '#value' => $group->gid,
    );
  }

  return $form;
}

/**
 * Group Delete Form.
 */
function pinapi_ui_group_delete_form(&$form_state, $group) {
  $form['gid'] = array(
    '#type' => 'hidden',
    '#value' => $group->gid,
  );

  return confirm_form($form,
    t('Are you sure you want to delete "%name"?', array('%name' => $group->name)),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/build/pinapi/groups',
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel')
  );
}

/**
 * Submit Callback: Group Edit Form.
 */
function pinapi_ui_group_form_submit($form, &$form_state) {
  $group = (object) $form_state['values'];

  pinapi_group_save($group);

  $form_state['redirect'] = 'admin/build/pinapi/groups';
}

/**
 * Submit Callback: Group Delete Form.
 */
function pinapi_ui_group_delete_form_submit($form, &$form_state) {
  if ( $form_state['values']['confirm'] ) {
    pinapi_group_delete(pinapi_group_load($form_state['values']['gid']));
    drupal_set_message(t('The group has been deleted.'));
  }
  $form_state['redirect'] = 'admin/build/pinapi/groups';
}

/**
 * Validation Callback: Group Delete Button.
 */
function pinapi_ui_group_form_delete_validate($form, &$form_state) {
  // Used to prevent default validation.
}

/**
 * Submit Callback: Group Delete Button.
 */
function pinapi_ui_group_form_delete_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/build/pinapi/group/' . $form_state['values']['gid'] . '/delete';
}

/**
 * Prize Administration Page.
 */
function pinapi_ui_view_prizes() {
  $links = array(
    'add' => l(t('Add Prize'), 'admin/build/pinapi/prize/add'),
  );
  
  $output = theme('item_list', $links);

  // Fetch all Pin API prizes.
  $prizes = pinapi_prize_get_prizes();
  if ( empty($prizes) ) {
    return $output . t('No prizes exist.');
  }

  $header = array(
    array('data' => t('Name')),
    array('data' => t('Operation')),
  );

  $rows = array();
  foreach ( $prizes as $prize ) {
    $operations = array(
      'edit' => l(t('Edit'), "admin/build/pinapi/prize/$prize->pid/edit"),
    );

    $rows[] = array(
      array('class' => 'pinapi-prize-name', 'data' => check_plain($prize->name)),
      array('class' => 'pinapi-operation', 'data' => implode(' | ', $operations)),
    );
  }

  $output .= theme('table', $header, $rows);
  $output .= theme('pager', NULL, 25);

  return $output;
}

/**
 * Prize Edit Form.
 */
function pinapi_ui_prize_form(&$form_state, $prize = NULL) {
  $format = 'm-d-Y';

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Machine Name'),
    '#default_value' => isset($prize->name) ? $prize->name : '',
    '#weight' => -1,
  );

  $form['quantity'] = array(
    '#type' => 'textfield',
    '#title' => t('Quantity'),
    '#default_value' => isset($prize->quantity) ? $prize->quantity : 0,
    '#size' => 7,
    '#weight' => 1,
  );

  $form['bias'] = array(
    '#type' => 'textfield',
    '#title' => t('Bias'),
    '#default_value' => isset($prize->bias) ? $prize->bias : 1,
    '#size' => 4,
    '#weight' => 2,
  );

  $xform['starts'] = array(
    '#type' => 'date_select',
    '#date_format' => $format,
    '#date_label_position' => 'within',
    '#date_year_range' => '0:+3',
    '#title' => t('Starts'),
    '#default_value' => isset($prize->starts) ? date_convert($prize->starts, DATE_UNIX, DATE_DATETIME) : 0,
    '#weight' => 3,
  );

  $xform['expires'] = array(
    '#type' => 'date_select',
    '#date_format' => $format,
    '#date_label_position' => 'within',
    '#date_year_range' => '0:+3',
    '#title' => t('Expires'),
    '#default_value' => isset($prize->expires) ? date_convert($prize->expires, DATE_UNIX, DATE_DATETIME) : 0,
    '#weight' => 4,
  );

  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Published'),
    '#default_value' => isset($prize->status) ? $prize->status : NULL,
    '#weight' => 5,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => isset($prize->pid) ? t('Update') : t('Add'),
    '#weight' => 20,
  );

  if ( isset($prize->pid) ) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#validate' => array('pinapi_ui_prize_form_delete_validate'),
      '#submit' => array('pinapi_ui_prize_form_delete_submit'),
      '#weight' => 21,
    );
    $form['pid'] = array(
      '#type' => 'hidden',
      '#value' => $prize->pid,
    );
  }

  return $form;
}

/**
 * Prize Delete Form.
 */
function pinapi_ui_prize_delete_form(&$form_state, $prize) {
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $prize->pid,
  );

  return confirm_form($form,
    t('Are you sure you want to delete "%name"?', array('%name' => $prize->name)),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/build/pinapi/groups',
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel')
  );
}

/**
 * Submit Callback: Prize Edit Form.
 */
function pinapi_ui_prize_form_submit($form, &$form_state) {
  $prize = (object) $form_state['values'];
  pinapi_prize_save($prize);
  $form_state['redirect'] = 'admin/build/pinapi/prizes';
}

/**
 * Submit Callback: Prize Delete Form.
 */
function pinapi_ui_prize_delete_form_submit($form, &$form_state) {
  if ( $form_state['values']['confirm'] ) {
    pinapi_prize_delete(pinapi_prize_load($form_state['values']['pid']));
    drupal_set_message(t('The prize has been deleted.'));
  }
  $form_state['redirect'] = 'admin/build/pinapi/prizes';
}

/**
 * Validation Callback: Prize Delete Button.
 */
function pinapi_ui_prize_form_delete_validate($form, &$form_state) {
  // Used to prevent default validation.
}

/**
 * Submit Callback: Prize Delete Button.
 */
function pinapi_ui_prize_form_delete_submit($form, &$form_state) {
  $form_state['redirect'] = 'admin/build/pinapi/prize/' . $form_state['values']['pid'] . '/delete';
}
