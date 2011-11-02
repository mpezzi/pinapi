<?php

/**
 * @file
 * Provides theme functions for pinapi_collect.module
 */


/**
 * Process variables for block-pinapi-collect.tpl.php
 */
function template_preprocess_pinapi_collect_block(&$vars) {
  $redeemed = $vars['account']->pinapi['redeemed'][$vars['group']->gid];
  $vars['content'] = $redeemed . '/' . variable_get('pinapi_collect_block_required', 15);
}
