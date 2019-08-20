<?php

/**
 * @file
 * Administration settings form.
 */

/**
 * Form definition.
 */
function care_taxonomy_admin_settings_form($form, &$form_state) {

  $form['imports'] = [
    '#type' => 'fieldset',
    '#title' => t('Import'),
    '#description' => t('Import types from CARE into Drupal.'),
  ];
  $form['imports']['import_mailing_suppressions'] = [
    '#type' => 'submit',
    '#value' => 'CARE mailing suppression code',
    '#submit' => [
      'care_taxonomy_admin_mailing_suppression_code_import',
    ],
  ];
  $form['imports']['import_membership_code'] = [
    '#type' => 'submit',
    '#value' => 'CARE membership code',
    '#submit' => [
      'care_taxonomy_admin_membership_code_import',
    ],
  ];
  $form['imports']['import_promotion_code'] = [
    '#type' => 'submit',
    '#value' => 'CARE promotion code',
    '#submit' => [
      'care_taxonomy_admin_promotion_code_import',
    ],
  ];
  $form['imports']['import'] = [
    '#type' => 'submit',
    '#value' => 'CARE source code',
    '#submit' => [
      'care_taxonomy_admin_sources_import',
    ],
  ];

  $form['activity_imports'] = [
    '#type' => 'fieldset',
    '#title' => t('Activity Imports'),
    '#description' => t('Import activity values from CARE into Drupal.'),
  ];

  $form['activity_imports']['activity_types'] = [
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Enabled Activity Types'),
  ];

  $data = [];
  $typedata = ['pLookupDataType' => 'xldtActivities'];
  $activities = care_call_method('GetLookupData', $data, $typedata);
  $activity_options = [];
  foreach ($activities as $activity) {
    $activity_options[(string) $activity->Activity] = (string) $activity->Activity . ' (' . (string) $activity->ActivityDesc . ')';
  }

  $activity_type_codes = variable_get('care_taxonomy_activities', []);
  $form['activity_imports']['activity_types']['care_taxonomy_activities'] = [
    '#title' => t('Activity types to import'),
    '#type' => 'checkboxes',
    '#options' => $activity_options,
    '#default_value' => $activity_type_codes,
  ];

  foreach ($activity_type_codes as $code) {
    $form['activity_imports'][$code] = [
      '#type' => 'submit',
      '#value' => $activity_options[$code],
      '#submit' => [
        'care_taxonomy_admin_activity_import',
      ],
    ];
  }

  $data = ['Activity' => 'MT'];
  $typedata = ['pLookupDataType' => 'xldtActivityValues'];
  $activity_types = care_call_method('GetLookupData', $data, $typedata);

  $form['links'] = [
    '#type' => 'fieldset',
    '#title' => t('Imported vocabularies'),
  ];

  $taxonomy_query = new EntityFieldQuery();
  $vocabularies = $taxonomy_query->entityCondition('entity_type', 'taxonomy_vocabulary')
    ->propertyCondition('machine_name','care_taxonomy_','STARTS_WITH')
    ->execute();
  foreach ($vocabularies['taxonomy_vocabulary'] as $vocabulary) {
    $vocab = taxonomy_vocabulary_load($vocabulary->vid);
    $vocab_link = l($vocab->name, 'admin/structure/taxonomy/' . $vocab->machine_name .'/care_codes');
    $edit_link = l(t('Edit'), 'admin/structure/taxonomy/' . $vocab->machine_name .'/edit');
    $links[] = "$vocab_link [$edit_link]";
  }

  $form['links']['links'] = [
    '#type' => 'markup',
    '#prefix' => '<ul><li>',
    '#markup' => implode('</li><li>', $links),
    '#suffix' => '</li></ul>',
  ];

  $form['array_filter'] = [
    '#type' => 'value',
    '#value' => TRUE,
  ];

  $form = system_settings_form($form);
  return $form;
}

/**
 * Import sources button callback.
 */
function care_taxonomy_admin_sources_import() {
  module_load_include('inc', 'care_taxonomy', 'care_taxonomy.rules');
  care_taxonomy_import_source_codes();
}

/**
 * Import promotion_code button callback.
 */
function care_taxonomy_admin_promotion_code_import() {
  module_load_include('inc', 'care_taxonomy', 'care_taxonomy.rules');
  care_taxonomy_import_promotion_codes();
}

/**
 * Import membership_code button callback.
 */
function care_taxonomy_admin_membership_code_import() {
  module_load_include('inc', 'care_taxonomy', 'care_taxonomy.rules');
  care_taxonomy_import_membership_codes();
}

/**
 * Import mailing_suppression_codes button callback.
 */
function care_taxonomy_admin_mailing_suppression_code_import() {
  module_load_include('inc', 'care_taxonomy', 'care_taxonomy.rules');
  care_taxonomy_import_mailing_suppression_codes();
}

/**
 * Import activity values button callback.
 */
function care_taxonomy_admin_activity_import($form, $form_state) {
  $activity_code = strtok($form_state['clicked_button']['#value'], ' ');
  care_taxonomy_import_activity_values($activity_code);
}
