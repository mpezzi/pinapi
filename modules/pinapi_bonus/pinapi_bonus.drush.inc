<?php 

/**
 * @file
 * Pin API Bonus Drush Integration.
 */


/**
 * Implements hook_drush_command().
 */
function pinapi_bonus_drush_command() {
  return array(
    'pinapi-bonus-generate' => array(
      'description' => 'Generate bonus codes',
      'arguments' => array(
        'from' => 'Start value',
        'to' => 'End value',
      ),
     'aliases' => array('pbg'),
    ),
  );
}

/**
 * Implements hook_drush_help().
 */
function pinapi_bonus_drush_help($section) {
  switch ( $section ) {
    case 'drush:pinapi-bonus-generate':
      return dt("Generate bonus codes.");
  }
}

/**
 * Drush Command Callback: Generate Bonus codes.
 */
function drush_pinapi_bonus_generate($from, $to) {
  $gid = drush_pinapi_group_choice('Choose the group you would like to generate for.', $gid);
  if ( !$gid ) {
    drush_die('Invalid group.');
  }
  $result = db_query_range("SELECT * FROM {pinapi_code} WHERE gid = %d", array($gid), $from, $to);
  while($code = db_fetch_object($result)) {
    $bonus = array(
      'cid' => $code->cid,
    );
    drupal_write_record('pinapi_bonus', $bonus);
  }
  drush_log(dt('Successfully generated bonus codes from !from to !to.', array('!from' => $from, '!to' => $to)), 'success');
}