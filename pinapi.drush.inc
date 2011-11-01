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
    'pinapi-code-verify' => array(
      'description' => 'Verify a code',
      'arguments' => array(
        'string' => 'A code string',
      ),
      'aliases' => array('pcv'),
    ),
    'pinapi-code-redeem' => array(
      'description' => 'Redeem a code',
      'arguments' => array(
        'string' => 'A code string or object',
      ),
      'aliases' => array('pcr'),
    ),
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
    'pinapi-code-import' => array(
      'description' => 'Import codes from a file.',
      'arguments' => array(
        'file' => 'A file to import the codes from.',
      ),
      'aliases' => array('pci'),
    ),
    'pinapi-code-export' => array(
      'description' => 'Export codes to a file.',
      'options' => array(
        '--file' => 'The filename of the output file.',
      ),
      'aliases' => array('pce'),
    ),
    'pinapi-code-clear' => array(
      'description' => 'Clear group codes from the database.',
      'arguments' => array(
        'gid' => 'A group id.',
      ),
      'aliases' => array('pcc'),
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
    case 'drush:pinapi-code-import':
      return dt("Import all code from file.");
    case 'drush:pinapi-code-export':
      return dt("Export all codes to file.");
    case 'drush:pinapi-code-clear':
      return dt("Clear codes from group.");
    case 'drush:pinapi-code-clear-all':
      return dt("Clear all codes.");
  }
}

/**
 * Drush Command Callback: Verify a code.
 */
function drush_pinapi_code_verify($string) {
  $group = (object) array(
    'gid' => 1,
    'status' => 1,
  );
  
  drush_print( pinapi_code_verify($string, $group) ? 'Yep' : 'Nope' );
}

/**
 * Drush Command Callback: Verify a code.
 */
function drush_pinapi_code_redeem($string) {
  $group = (object) array(
    'gid' => 1,
    'status' => 1,
  );
  
  drush_print( pinapi_code_redeem($string, $group) ? 'Yep' : 'Nope' );
}

/**
 * Drush Command Callback: Generate codes for a prize pool.
 */
function drush_pinapi_code_generate($amount = NULL, $gid = NULL) {

  // Insert Options.
  $limit = 100000;

  // Ensure there is an amount set.
  if ( is_null($amount) ) {
    drush_die('No amount set. Aborting.');
  }

  // @TODO: Get group, make sure it is valid.
  $gid = drush_pinapi_group_choice('Choose the group you would like to generate for.', $gid);
  if ( !$gid ) {
    drush_die('Invalid group.');
  }
  
  // Get code generator.
  $type = drush_get_option('type') ? drush_get_option('type') : NULL;
  $generator = pinapi_code_get_generator($type);
  if ( !$generator ) {
    drush_die('Invalid code generator.');
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

    drush_print("Group: " . $gid);
    drush_print('Amount: ' . $amount);
    drush_print('Limit: ' . $limit);
    drush_print('Chunks: ' . $chunks);
    drush_print('Left Overs: ' . $leftover);

    drush_print(dt('Generating !amount codes.', array('!amount' => number_format($amount))));

    // Import codes in chunks to improve SQL performance.
    if ( $amount >= $limit ) {
      for ( $a = 1; $a <= $chunks; $a++ ) {

        // Insert a chunks of codes using one insert sql query.
        pinapi_code_generate_insert($generator, $limit, $gid);
        drush_pinapi_code_generate_status($limit, $limit * $a);

        // Insert leftover codes using one insert sql query after final chunk.
        if ( $a == $chunks && $leftover > 0 ) {
          pinapi_code_generate_insert($generator, $leftover, $gid);
          drush_pinapi_code_generate_status($leftover, $amount);
        }

      }
    }
    // Amount is less then our limit, perform only one insert.
    else {
      pinapi_code_generate_insert($generator, $amount, $gid);
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

/**
 * Display a status update for code generation.
 */
function drush_pinapi_code_generate_status($imported, $total) {
  $vars = array(
    '!imported' => format_plural($imported, '@count code', '@count codes'),
    '!total' => format_plural($total, '@count code', '@count codes'),
  );
  
  drush_print(dt('Inserted !imported, !total in total.', $vars));
}

/**
 * Drush Command Callback: Import codes for a group.
 */
function drush_pinapi_code_import() {
  // @TODO: import codes from a file, or series of files.
}

/**
 * Drush Command Callback: Export codes for a group.
 */
function drush_pinapi_code_export() {
  $limit = 1000000;

  // @TODO: Get group, make sure it is valid.
  $gid = drush_pinapi_group_choice('Choose the group you would like to export for.', $gid);
  if ( !$gid ) {
    drush_die('Invalid group.');
  }

  // Get a amount of codes in database.
  $amount = db_result(db_query("SELECT COUNT(*) FROM {pinapi_code} WHERE gid = %d", $gid));

  if ( drush_confirm(dt('Are you sure you want to export !amount codes?', array('!amount' => number_format($amount)))) ) {
    timer_start('drush_pinapi_code_export');

    // Split up amount to improve SQL performance.
    $chunks = floor($amount / $limit);
    $leftover = $amount - ( $limit * $chunks );

    // Export codes in chunks to improve SQL performance.
    if ( $amount >= $limit ) {
      for ( $a = 1; $a <= $chunks; $a++ ) {
        // Insert a chunks of codes using one insert sql query.
        $output = '';
        $result = db_query_range("SELECT * FROM {pinapi_code} WHERE gid = %d", array($gid), $limit * ( $a - 1 ), $limit);
        while ( $row = db_fetch_object($result) ) {
          $output .= $row->code . "\n";
        }

        if ( file_put_contents('pinapi-export-' . $a . '.txt', $output) ) {
          drush_log(dt('Export complete.'), 'success');
        }
        else {
          drush_log(dt('Export failed.'), 'failed');
          drush_die('Aborting.');
        }
        //pinapi_code_generate_insert($generator, $limit, $gid);
        //drush_pinapi_code_generate_status($limit, $limit * $a);

        // Insert leftover codes using one insert sql query after final chunk.
        if ( $a == $chunks && $leftover > 0 ) {
          // Insert a chunks of codes using one insert sql query.
          $output = '';
          $result = db_query_range("SELECT * FROM {pinapi_code} WHERE gid = %d", array($gid), $amount - $leftover, $leftover);
          while ( $row = db_fetch_object($result) ) {
            $output .= $row->code . "\n";
          }

          if ( file_put_contents('pinapi-export-' . ( $a + 1 ) . '.txt', $output) ) {
            drush_log(dt('Export complete.'), 'success');
          }
          else {
            drush_log(dt('Export failed.'), 'failed');
            drush_die('Aborting.');
          }
          //pinapi_code_generate_insert($generator, $leftover, $gid);
          //drush_pinapi_code_generate_status($leftover, $amount);
        }

      }
    }
    // Amount is less then our limit, perform only one insert.
    else {
      drush_print(dt('Limit: ' . $amount));
      drush_print(dt('Offset: ' . 0));
      // Insert a chunks of codes using one insert sql query.
      $output = '';
      $result = db_query("SELECT * FROM {pinapi_code} WHERE gid = %d", $gid);
      while ( $row = db_fetch_object($result) ) {
        $output .= $row->code . "\n";
      }

      if ( file_put_contents('pinapi-export-0.txt', $output) ) {
        drush_log(dt('Export complete.'), 'success');
      }
      else {
        drush_log(dt('Export failed.'), 'failed');
        drush_die('Aborting.');
      }
      //pinapi_code_generate_insert($generator, $amount, $gid);
      //drush_pinapi_code_generate_status($amount, $amount);
    }

    // Stop the timer.
    $time = timer_stop('drush_pinapi_code_export');
    $duration = $time['time'];

    drush_log(dt('Successfully exported !amount codes in !duration', array('!amount' => number_format($amount), '!duration' => $duration)), 'success');
  }
  else {
    drush_die('Aborting.');
  }
}

/**
 * Drush Command Callback: Clear codes from group.
 */
function drush_pinapi_code_clear($gid = NULL) {
  if ( $gid = drush_pinapi_group_choice('Choose the group you would like to clear.', $gid) ) {
    if ( drush_confirm(dt('Are you sure you want to clear this group?')) ) {
      pinapi_code_clear($gid);
      drush_log(dt('Cleared all group codes.'), 'success');
    }
  }
  else {
    drush_die('Aborting.');
  }
}

/**
 * Drush Command Callback: Clear all codes.
 */
function drush_pinapi_code_clear_all() {
  if ( drush_confirm(dt('Are you sure you want to remove all codes?')) ) {
    pinapi_code_clear_all();
    drush_log(dt('Cleared all codes.'), 'success');
  }
  else {
    drush_die('Aborting.');
  }
}

/**
 * Return a list of choices for groups.
 *
 * @param $gid
 *   A group id.
 * @return
 *   A valid group id.
 */
function drush_pinapi_group_choice($prompt, $gid = NULL) {
  $choices = array();
  foreach ( pinapi_group_get_groups() as $group ) {
    $choices[$group->name] = $group->gid;
  }

  if ( is_null($gid) ) {
    $gid = drush_choice($choices, $prompt, '!key');
  }

  return isset($choices[$gid]) ? $choices[$gid] : FALSE;
}
