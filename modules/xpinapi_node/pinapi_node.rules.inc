<?php 

/**
 * @file
 * Rules specific functions that expose Pin API.
 */

/**
 * Implements hook_rules_event_info().
 */
function pinapi_node_rules_event_info() {
  return array(
    'pinapi_node_prize_award' => array(
      'label' => t('User is awarded a node'),
      'module' => 'Pin API',
      'arguments' => array(
        'user' => array('type' => 'user', 'label' => t('User that was awarded')),
        'node' => array('type' => 'node', 'label' => t('Node that was awarded')),
      ),
    ),
  );
}
