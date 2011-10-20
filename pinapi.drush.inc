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
        'length' => 'The length of the generated codes.',
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
function drush_pinapi_code_generate($amount = 0, $length = 12) {

  // SQL Insert Options.
  $limit = 100000;

  if ( $amount == 0 ) {
    drush_die('No amount set. Aborting.');
  }

  if ( drush_confirm(dt('Are you sure you want to create !amount codes?', array('!amount' => number_format($amount)))) ) {

    // Split up amount to improve SQL performance.
    $chunks = floor($amount / $limit);
    $leftover = $amount - ( $limit * $chunks );

    // Set options for generator.
    $options = array(
      'salt' => variable_get('pinapi_code_generate_salt', '123456789'),
      'length' => $length,
    );

    drush_print('Limit: ' . $limit);
    drush_print('Amount: ' . $amount);
    drush_print('Chunks: ' . $chunks);
    drush_print('Left Overs: ' . $leftover);
    drush_print(dt('Generating !amount codes.', array('!amount' => number_format($amount))));

    // Import codes in chunks to improve SQL performance.
    if ( $amount >= $limit ) {
      for ( $a = 1; $a <= $chunks; $a++ ) {
        $sql = "INSERT INTO {pinapi_code} (code, pool_id) VALUES ";

        for ( $b = 0; $b < $limit; $b++ ) {
          $sql .= ( $b != 0 ) ? ', ' : '';
          $sql .= "('" . pinapi_code_generate('generic', $options) . "', 1)";
        }

        db_query($sql);
        drush_print(dt('Generated !current codes.', array('!current' => number_format($b * $a))));

        // Import left overs after final chunk.
        if ( $a == $chunks && $leftover > 0 ) {
          $sql = "INSERT INTO {pinapi_code} (code, pool_id) VALUES ";

          for ( $b = 0; $b < $leftover; $b++ ) {
            $sql .= ( $b != 0 ) ? ', ' : '';
            $sql .= "('" . pinapi_code_generate('generic', $options) . "', 1)";
          }

          db_query($sql);
          drush_print(dt('Generated !current codes.', array('!current' => number_format($leftover))));
        }
      }
    }
    else {
      $sql = "INSERT INTO {pinapi_code} (code, pool_id) VALUES ";

      for ( $b = 0; $b < $amount; $b++ ) {
        $sql .= ( $b != 0 ) ? ', ' : '';
        $sql .= "('" . pinapi_code_generate('generic', $options) . "', 1)";
      }

      db_query($sql);
      drush_print(dt('Generated !current codes.', array('!current' => number_format($amount))));
    }

    drush_log(dt('Successfully generated !amount codes.', array('!amount' => number_format($amount))), 'success');
  }
  else {
    drush_die('Aborting.');
  }
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
