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
      'description' => 'Mark codes as bonus.',
      'arguments' => array(
        'offset' => 'Offset',
        'limit' => 'Number of codes',
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
      return dt("Mark codes as bonus.");
  }
}

/**
 * Drush Command Callback: Generate Bonus codes.
 */
function drush_pinapi_bonus_generate($offset, $limit) {
  $gid = drush_pinapi_group_choice('Choose the group you would like to generate for.', $gid);
  if ( !$gid ) {
    drush_die('Invalid group.');
  }

  // start the timer
  timer_start('drush_pinapi_bonus_generate');

  // Mark certain codes as bonus.
  $result = db_query_range("SELECT * FROM {pinapi_code} WHERE gid = %d AND status = 1", array($gid), $offset, $limit);
  while ( $code = db_fetch_object($result) ) {
    $bonus = (object) array('cid' => $code->cid);
    pinapi_bonus_save($bonus);
  }

  $time = timer_stop('drush_pinapi_bonus_generate');
  $duration = $time['time'];

  drush_log(dt('Successfully generated bonus codes from !from to !to in !duration.', array('!from' => $offset, '!to' => ($offset + $limit), '!duration' => $duration)), 'success');
}
