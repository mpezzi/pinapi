<?php

/**
 * @file
 * PinAPI Module Drush Integration.
 */

/**
 * Implements hook_drush_command().
 */
function pinapi_drush_command() {
  return array(
    'pinapi-code-generate' => array(
      'description' => 'Generate codes for a prize pool.',
      'arguments' => array(
        'amount' => 'Amount of codes to generate.',
        'length' => 'The length of the generated code.',
      ),
      'examples' => array(
        'drush pinapi-code-generate 1000000 12' =>
          "Generate codes for a pool",
      ),
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
      return dt("Generate codes for a prize pool.");
    case 'drush:pinapi-code-clear-all':
      return dt("Clear all codes from the database.");
    case 'drush:pinapi-code-export':
      return dt("Export all codes to file.");
    case 'drush:pinapi-code-import':
      return dt("Import all code from file.");
  }
}

/**
 * Generate codes for a prize pool.
 */
function drush_pinapi_code_generate($amount = 0, $length = 12) {
  $salt = variable_get('pinapi_code_generate_salt', '');
  $amount_formatted = number_format($amount);

  $limit = 500000;

  if ( drush_confirm(dt('Do you really want to generate !amount codes?', array('!amount' => $amount_formatted))) ) {
    drush_print(dt('Generating !amount codes.', array('!amount' => $amount_formatted)));

    $chunks = ceil($amount / $limit);
    
    db_query('START TRANSACTION');
    for ( $a = 0; $a < $chunks; $a++ ) {
      $sql = 'INSERT INTO {pinapi_code} (code, pool_id) VALUES ';

      for ( $i = 0; $i < $limit; $i++ ) {
        if ( $i != 0 ) {
          $sql .= ', ';
        }
        $sql .= "('" . strtoupper(substr(sha1(time() . $salt . rand()), 0, $length)) . "', 1)";
      }

      db_query($sql);
      drush_print(dt('Generated !status codes.', array('!status' => number_format($i * $a))));
      
      //sleep(5);
    }
    
    db_query("COMMIT");

    drush_log(dt('Successfully generated !amount codes.', array('!amount' => $amount_formatted)), 'success');
  }
  else {
    drush_die('Aborting.');
  }
}

/**
 * Import codes from a file.
 */
function drush_pinapi_code_import() {
  $args = func_get_args();
  if ( !isset($args[0]) ) {
    drush_set_error('DRUSH_VARIABLE_ERROR', dt('No file specified'));
  }
  
  
  
  drush_print($args[0]);
}

/**
 * Export codes to a file.
 */
function drush_pinapi_code_export() {
  $limit = 1000000;

  $total = db_result(db_query("SELECT COUNT(*) FROM {pinapi_code}"));
  //$total = 500000;
  $chunks = ceil($total / $limit);

  $filename = drush_get_option('file');
  $name = basename($filename);

  drush_print(ceil($total / $limit));
  //drush_die('Abort!');

  for ( $i = 1; $i <= $chunks; $i++ ) {
    $output = '';
    
    $result = db_query("SELECT * FROM {pinapi_code} LIMIT %d, %d", ( $i - 1 ) * $limit, $limit);
    while ( $row = db_fetch_object($result) ) {
      $output .= $row->code . "\n";
    }

    //if ( $filename ) {
      //if ( file_exists($filename) ) {
      //  if ( !drush_confirm(dt("File $filename exists. Do you really want to overwrite?")) ) {
      //    return;
      //  }
      //}

      if ( file_put_contents('pins-' . $i . '.txt', $output) ) {
        drush_log(dt('Export complete.'), 'success');
      }
      else {
        drush_log(dt('Export failed.'), 'failed');
      }
    //}
    //else {
      //drush_print_r($output);
    //}
  }
}

/**
 * Clear all codes from pool.
 */
function drush_pinapi_code_clear($pool_id = NULL) {
  
}

/**
 * Clear all codes from the database.
 */
function drush_pinapi_code_clear_all() {
  if ( drush_confirm(dt('Are you sure you want to remove all codes from the database?')) ) {
    db_query("TRUNCATE TABLE {pinapi_code}");
    drush_log(dt('Cleared all codes from database.'), 'success');
  }
  else {
    drush_die('Aborting.');
  }
}