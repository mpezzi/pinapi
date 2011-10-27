<?php 

/**
 * @file
 * Provides theme functions for Pin API Node module.
 */

/**
 * Theme PinAPI Administration Build Form.
 *
 * @ingroup themeable
 */
function theme_pinapi_node_pool_prize_form($form) {
  drupal_add_css(drupal_get_path('module', 'pinapi_node') . '/pinapi_node.css');
  
  $header = array(
    array(),
    array('data' => t('Type')),
    array('data' => t('Name')),
    array('data' => t('Quantity')),
    t('Operations'),
  );
  
  if ( isset($form['pid']) && is_array($form['pid']) ) {
    foreach ( element_children($form['pid']) as $key ) {
      $rows[] = array(
        array('data' => drupal_render($form['status'][$key]), 'width' => 20),
        array('data' => drupal_render($form['content_type'][$key]), 'width' => 40),
        drupal_render($form['name'][$key]),
        array('data' => drupal_render($form['quantity'][$key]), 'width' => 40),
        array('data' => drupal_render($form['operations'][$key]), 'width' => 120),
      );
    }
  }
  else {
    $rows[] = array(array('data' => t('No prizes available.'), 'colspan' => 5));
  }
  
  return drupal_render($form['locked_message']) . theme('table', $header, $rows) . drupal_render($form);
}
