<?php 

/**
 * @file
 * Pin API Drush Integration.
 */


/**
 * Implements hook_drush_command().
 */
function pinapi_drush_command() {
  return array(
    'pinapi-code-generate' => array(
      'description' => 'Generate codes for a pool',
      'arguments' => array(
        'amount' => 'Amount of codes to generate.',
      ),
      'options' => array(
        '--type' => 'The type of generator to use to create the codes.',
        '--length' => 'The length of the code to generate.',
      ),
      'aliases' => array('pcg'),
    ),
    'pinapi-code-clear-all' => array(
      'description' => 'Clear all codes from the database.',
      'aliases' => array('pcca'),
    ),
  );
}

/**
 * Implements hook_drush_help().
 */
function pinapi_drush_help($section) {
  switch ( $section ) {
    case 'drush:pinapi-code-generate':
      return dt("Generate codes for a pool.");
    case 'drush:pinapi-code-clear':
      return dt("Clear codes from the database.");
  }
}

/**
 * Drush Command Callback.
 *
 * Generate codes for a prize pool.
 */
function drush_pinapi_code_generate($amount = 0) {

  // Insert Options.
  $limit = 100000;

  // Drush Options.
  $type = drush_get_option('type') ? drush_get_option('type') : NULL;

  // Get current system code generator.
  $generator = pinapi_code_get_generator($type);

  // If code generator doesn't exist.
  if ( !$generator ) {
    drush_die('Invalid code generator.');
  }

  // Ensure there is an amount set.
  if ( $amount == 0 ) {
    drush_die('No amount set. Aborting.');
  }

  if ( drush_confirm(dt('Are you sure you want to create !amount codes?', array('!amount' => number_format($amount)))) ) {

    // Calculate generating time.
    timer_start('drush_pinapi_code_generate');

    // Split up amount to improve SQL performance.
    $chunks = floor($amount / $limit);
    $leftover = $amount - ( $limit * $chunks );

    if ( drush_get_option('length') ) {
      $generator['options']['length'] = drush_get_option('length');
    }

    drush_print('Amount: ' . $amount);
    drush_print('Limit: ' . $limit);
    drush_print('Chunks: ' . $chunks);
    drush_print('Left Overs: ' . $leftover);

    drush_print(dt('Generating !amount codes.', array('!amount' => number_format($amount))));

    // Import codes in chunks to improve SQL performance.
    if ( $amount >= $limit ) {
      for ( $a = 1; $a <= $chunks; $a++ ) {

        // Insert a chunks of codes using one insert sql query.
        db_query(pinapi_code_generate_insert_query($generator, $limit));
        drush_pinapi_code_generate_status($limit, $limit * $a);

        // Insert leftover codes using one insert sql query after final chunk.
        if ( $a == $chunks && $leftover > 0 ) {
          db_query(pinapi_code_generate_insert_query($generator, $leftover));
          drush_pinapi_code_generate_status($leftover, $amount);
        }

      }
    }
    // Amount is less then our limit, perform only one insert.
    else {
      db_query(pinapi_code_generate_insert_query($generator, $amount));
      drush_pinapi_code_generate_status($amount, $amount);
    }

    // Stop the timer.
    $time = timer_stop('drush_pinapi_code_generate');
    $duration = $time['time'];

    drush_log(dt('Successfully generated !amount codes in !duration.', array('!amount' => number_format($amount), '!duration' => $duration)), 'success');
  }
  else {
    drush_die('Aborting.');
  }
}

function drush_pinapi_code_generate_status($imported, $total) {
  $vars = array(
    '!imported' => format_plural($imported, '@count code', '@count codes'),
    '!total' => format_plural($total, '@count code', '@count codes'),
  );
  
  drush_print(dt('Inserted !imported, !total in total.', $vars));
}

/**
 * Drush Command Callback.
 *
 * Clear all codes.
 */
function drush_pinapi_code_clear_all($type = NULL) {
  if ( drush_confirm(dt('Are you sure you want to remove all codes?')) ) {
    pinapi_code_clear_all();
    drush_log(dt('Cleared all codes.'), 'success');
  }
  else {
    drush_die('Aborting.');
  }
}
