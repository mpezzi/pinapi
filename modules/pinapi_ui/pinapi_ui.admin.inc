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
 * Adminstration Page.
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
  );

  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Published'),
    '#default_value' => isset($group->status) ? $group->status : NULL,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => isset($group->gid) ? t('Update') : t('Add'),
  );

  if ( isset($group->gid) ) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#validate' => array('pinapi_ui_group_form_delete_validate'),
      '#submit' => array('pinapi_ui_group_form_delete_submit'),
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
    pinapi_group_delete($form_state['values']['gid']);
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
 * Adminstration Page.
 */
function pinapi_ui_view_prizes() {
  return __FUNCTION__;
}

