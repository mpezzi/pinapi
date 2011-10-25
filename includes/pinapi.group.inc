<?php

/**
 * @file
 * Provides a generalized API for Pin API Groups.
 */


/**
 * Load a Pin API Group Object.
 *
 * @param $gid
 *   A group id.
 * @param $reset
 *   Whether to reset the internal pinapi_group_load cache.
 * @return
 *   Group object matching the group id.
 */
function pinapi_group_load($gid, $reset = FALSE) {
  static $groups = array();

  // Reset internal static cache.
  if ( $reset ) {
    $groups = array();
  }

  // Return the statically cached group.
  if ( isset($groups[$gid]) ) {
    return is_object($groups[$gid]) ? drupal_clone($groups[$gid]) : $groups[$gid];
  }

  // Retrieve the group object.
  $group = db_fetch_object(db_query("SELECT * FROM {pinapi_group} WHERE gid = %d", $gid));

  if ( $group && $group->gid ) {
    // Invoke hook_pinapi_groupapi operation in all modules and attach results to the group object.
    if ( $extra = pinapi_groupapi_invoke('load', $group) ) {
      foreach ( $extra as $key => $value ) {
        $group->$key = $value;
      }
    }

    // Store group object to internal static cache.
    $groups[$group->gid] = is_object($group) ? drupal_clone($group) : $group;
  }

  return $group;
}

/**
 * Save a Pin API Group Object.
 */
function pinapi_group_save(&$group) {
  // Invoke hook_pinapi_groupapi operation in all modules.
  pinapi_groupapi_invoke('presave', $group);

  // Determine whether this is a new group.
  $group->is_new = empty($group->gid);

  if ( $group->is_new ) {
    drupal_write_record('pinapi_group', $group);
    $op = 'insert';
  }
  else {
    drupal_write_record('pinapi_group', $group, 'gid');
    $op = 'update';
  }

  // Invoke hook_pinapi_groupapi operation in all modules.
  pinapi_groupapi_invoke($op, $group);

  // Clear the page and blocks cache.
  cache_clear_all();
}

/**
 * Invoke a hook_pinapi_groupapi operation in all modules.
 *
 * @param
 *   A group object.
 * @param
 *   A string containing the name of the groupapi operation.
 * @return
 *   The returned value of the invoked hooks.
 */
function pinapi_groupapi_invoke($op, &$group) {
  $return = array();
  foreach ( module_implements('pinapi_groupapi') as $name ) {
    $function = $name . '_pinapi_groupapi';
    $result = $function($op, $group);
    if ( isset($result) && is_array($result) ) {
      $return = array_merge($return, $result);
    }
    else {
      $return[] = $result;
    }
  }
  return $return;
}

/**
 * Return a list of all groups.
 *
 * @param $reset
 *   Whether to reset the internal pinapi_group_get_groups cache.
 * @return
 *   An array of group objects.
 */
function pinapi_group_get_groups($reset = FALSE) {
  static $groups;

  if ( $reset ) {
    $groups = array();
  }

  if ( !isset($groups) ) {
    $groups = array();
    $result = db_query(db_rewrite_sql("SELECT gid FROM {pinapi_group}"));
    while ( $row = db_fetch_object($result) ) {
      $groups[$row->gid] = pinapi_group_load($row->gid, $reset);
    }
  }

  return $groups;
}
