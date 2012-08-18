<?php

/**
 * @file
 * Provides helper functions for calculating totals.
 */


/**
 * Set Pin API Cache Value.
 */
function pinapi_cache_set($uid, $cid, $name, $value) {
  $query = db_merge('pinapi_cache')
    ->key(array('uid' => $uid, 'cid' => $cid, 'name' => $name))
    ->fields(array('updated' => time(), 'value' => $value))
    ->execute();

  return $query;
}

/**
 * Increase Pin API Cache Value.
 */
function pinapi_cache_inc($uid, $cid, $name, $value = 1) {
  $query = db_merge('pinapi_cache')
    ->key(array('uid' => $uid, 'cid' => $cid, 'name' => $name))
    ->fields(array('updated' => time(), 'value' => $value))
    ->expression('value', 'value + :inc', array(':inc' => $value))
    ->execute();

  return $query;
}

/**
 * Get Pin API Cache Value.
 */
function pinapi_cache_get($uid, $cid, $name, $default = NULL) {
  $result = db_select('pinapi_cache', 's')
    ->fields('s', array('value'))
    ->condition('uid', $uid)
    ->condition('cid', $cid)
    ->condition('name', $name)
    ->execute()
    ->fetchField();

  return $result ? $result : $default;
}

/**
 * Delete a Pin API Cache Value.
 */
function pinapi_cache_del($uid, $cid, $name) {
  $query = db_delete('pinapi_cache')
    ->condition('uid', $uid)
    ->condition('cid', $cid)
    ->condition('name', $name)
    ->execute();

  return $query;
}