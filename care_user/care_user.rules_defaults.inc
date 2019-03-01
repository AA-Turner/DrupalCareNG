<?php

/**
 * @file
 * Default Rule definitions.
 */

/**
 * Implements hook_default_rules_configuration().
 */
function care_user_default_rules_configuration() {
  $rules = array();
  foreach (glob(__DIR__ . '/rules/*.json') as $rule_filename) {
    $rule_name = basename($rule_filename, '.json');
    $rules[$rule_name] = rules_import(file_get_contents($rule_filename));
  }
  return $rules;
}
