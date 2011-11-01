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
  foreach ( $vars['gids'] as $gid ) {
    $vars['toolbars'][] = theme('pinapi_debug_group', pinapi_group_load($gid));
  }
}

/**
 * Implements template_preprocess_pinapi_debug_group().
 */
function template_preprocess_pinapi_debug_group(&$vars) {
  $group = $vars['group'];

  /*
  $availability = array();
  $prizes = pinapi_pool_prizes($pool_id);
  foreach ( $prizes as $prize ) {
    $availability[] = $prize['name'] . ' (' . $prize['quantity'] . ' left x' . $prize['bias'] . ')';
  }
  */

  //$total = db_result(db_query("SELECT COUNT(*) FROM {pinapi_code} WHERE gid = %d AND status = 1", $group->gid));

  $codes = array();
  $result = db_query("SELECT * FROM {pinapi_code} WHERE gid = %d AND status = 1 LIMIT 30", $group->gid);
  while ( $code = db_fetch_object($result) ) {
    $codes[] = '<span class="pin" data-code="' . $code->code . '">' . $code->code . ' x' . $code->quantity . '</span>';
  }

  /*
  $content['prizes'] = array(
    '#type' => 'item',
    '#title' => t('Prizes'),
    '#value' => theme('item_list', $availability),
    '#weight' => 0,
  );
  */

  $content['codes'] = array(
    '#type' => 'item',
    '#prefix' => '<div class="codes" data-pool="' . $group->gid . '">',
    '#suffix' => '</div>',
    '#title' => t('Codes'),
    '#value' => implode(', ', $codes),
    '#weight' => 1,
  );

  $vars['debug'] = drupal_render($content);
}
