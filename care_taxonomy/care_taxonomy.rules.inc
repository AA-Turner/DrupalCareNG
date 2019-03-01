<?php

/**
 * Implements hook_rules_action_info().
 */
function care_taxonomy_rules_action_info() {
  $actions = [];
  $actions['care_taxonomy_import_source_codes'] = [
    'label' => 'Import Source codes from CARE',
    'group' => 'CARE Taxonomy',
  ];
  $actions['care_taxonomy_import_promotion_codes'] = [
    'label' => 'Import Promotion codes from CARE',
    'group' => 'CARE Taxonomy',
  ];
  $actions['care_taxonomy_import_membership_codes'] = [
    'label' => 'Import Membership codes from CARE',
    'group' => 'CARE Taxonomy',
  ];
  $actions['care_taxonomy_import_mailing_suppression_codes'] = [
    'label' => 'Import Mailing suppression codes from CARE',
    'group' => 'CARE Taxonomy',
  ];
  return $actions;
}

