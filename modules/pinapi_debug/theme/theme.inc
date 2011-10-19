<?php 

/**
 * @file
 * Provides theme functions for Pin API Debug module.
 */


/**
 * Implements template_preprocess_pinapi_debug_toolbar().
 */
function template_preprocess_pinapi_debug_toolbar(&$vars) {
  $vars['toolbars'] = array();
  foreach ( $vars['pools'] as $pool_id ) {
    $vars['toolbars'][] = theme('pinapi_debug_pool', $pool_id);
  }
}

/**
 * Implements template_preprocess_pinapi_debug_pool().
 */
function template_preprocess_pinapi_debug_pool(&$vars) {
  $pool_id = $vars['pool_id'];
  
  $availability = array();
  $prizes = pinapi_pool_prizes($pool_id);
  foreach ( $prizes as $prize ) {
    $availability[] = $prize['name'] . ' (' . $prize['quantity'] . ' left x' . $prize['bias'] . ')';
  }
  
  $total = db_result(db_query("SELECT COUNT(*) FROM {pinapi_code} WHERE pool_id = %d AND status = 1", $pool_id));
  
  $codes = array();
  $result = db_query("SELECT * FROM {pinapi_code} WHERE pool_id = %d AND status = 1 LIMIT %d, 30", $pool_id, rand(0, $total));
  while ( $code = db_fetch_array($result) ) {
    $codes[] = '<span class="pin" data-code="' . $code['code'] . '">' . $code['code'] . ' x' . $code['quantity'] . '</span>';
  }

  $content['prizes'] = array(
    '#type' => 'item',
    '#title' => t('Prizes'),
    '#value' => theme('item_list', $availability),
    '#weight' => 0,
  );

  $content['pins'] = array(
    '#type' => 'item',
    '#prefix' => '<div class="pins" data-pool="' . $pool_id . '">',
    '#suffix' => '</div>',
    '#title' => t('Pins'),
    '#value' => implode(', ', $codes),
    '#weight' => 1,
  );
  
  $vars['debug'] = drupal_render($content);
}