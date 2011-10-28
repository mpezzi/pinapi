<?php 

/**
 * @file
 * Provides Pin API Node Administrative Forms.
 */


/**
 * Pin API Nodes Administrative Settings Form.
 */
function pinapi_node_settings_form() {
  $form['pinapi_group'] = array(
    '#type' => 'fieldset',
    '#title' => t('Group'),
    '#description' => t('Settings for groups'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
  );

  $form['pinapi_group']['pinapi_node_group_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Group Node Types'),
    '#description' => t('Content types that can have prizes associated to them.'),
    '#options' => node_get_types('names'),
    '#default_value' => variable_get('pinapi_node_group_types', array()),
  );

  // These options is only available when pinapi_prize is enabled.
  if ( module_exists('pinapi_prize') ) {
    $form['pinapi_prize'] = array(
      '#type' => 'fieldset',
      '#title' => t('Prize'),
      '#description' => t('Settings for prizes'),
      '#collapsed' => FALSE,
      '#collapsible' => FALSE,
    );

    $form['pinapi_prize']['pinapi_node_prize_types'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Prize Node Types'),
      '#description' => t('Content types that can be redeemed as a prize.'),
      '#options' => node_get_types('names'),
      '#default_value' => variable_get('pinapi_node_prize_types', array()),
    );
  }

  return system_settings_form($form);
}

/**
 * Page callback for autocomplete.
 */
function pinapi_node_autocomplete() {
  $type = 'prize';
  $types = pinapi_node_types($type);

  $result = db_query("SELECT nid, title FROM {node} WHERE type IN ('" . implode("', '", $types) . "')");
  while ( $node = db_fetch_object($result) ) {
    $nodes[$node->nid] = $node->title;
  }

  drupal_json($nodes);
}
