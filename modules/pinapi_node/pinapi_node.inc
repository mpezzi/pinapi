<?php 

/**
 * @file
 * Provide internal helper functions for Pin API Node module.
 */

/**
 * Returns an array of Pin API enabled node types.
 *
 * @param $type
 *   The type of node that should be checked.
 * @param $node_type
 *   The node type to be checked.
 * @return
 *   An array of enabled node types or boolean value if $node_type is set.
 */
function pinapi_node_types($pinapi_type, $node_type = NULL) {
  static $types;

  // Collect node types from variable.
  if ( !isset($types[$pinapi_type]) ) {
    $types = array();
    foreach ( variable_get('pinapi_node_' . $pinapi_type . '_types', array()) as $type ) {
      if ( $type ) {
        $types[$pinapi_type][] = $type;
      }
    }
  }

  // Argument was given, return TRUE or FALSE.
  if ( !is_null($node_type) && isset($types[$pinapi_type]) ) {
    return in_array($node_type, $types[$pinapi_type]);
  }

  return isset($types[$pinapi_type]) ? $types[$pinapi_type] : array();
}
