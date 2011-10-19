<?php 

/**
 * @file
 * Provides hook documentation for the PinAPI module.
 */

/**
 * Implements hook_pinapi_prize_load().
 */
function hook_pinapi_prize_load(&$prize) {
  
}

/**
 * Implements hook_pinapi_prize_insert().
 */
function hook_pinapi_prize_insert(&$prize) {
  
}

/**
 * Implements hook_pinapi_prize_update().
 */
function hook_pinapi_prize_update(&$prize) {
  
}

/**
 * Implements hook_pinapi_prize_delete().
 */
function hook_pinapi_prize_delete($prize) {
  
}

/**
 * Implements hook_pinapi_prize_availability().
 */
function hook_pinapi_prize_availability(&$prize, $pool_id, $account) {
  
}

/**
 * Implements hook_pinapi_pool_load().
 */
function hook_pinapi_pool_load(&$pool) {
  
}

/**
 * Implements hook_pinapi_pool_insert().
 */
function hook_pinapi_pool_insert(&$pool) {
  
}

/**
 * Implements hook_pinapi_pool_update().
 */
function hook_pinapi_pool_update(&$pool) {
  
}

/**
 * Implements hook_pinapi_pool_delete().
 */
function hook_pinapi_pool_delete($pool) {
  
}

/**
 * Implements hook_pinapi_pool_prizes_alter().
 */
function hook_pinapi_pool_prizes_alter(&$prizes, $pool_id, $account) {
  
}

/**
 * Implements hook_pinapi_code_generator().
 */
function hook_pinapi_code_generator() {
  return array(
    'md5' => array(
      'name' => t('MD5'),
      'callback' => 'pinapi_code_generate_md5',
    ),
    'md5_timestamp' => array(
      'name' => t('MD5 with Timestamp'),
      'callback' => 'pinapi_code_generate_md5_timestamp',
    ),
  );
}
