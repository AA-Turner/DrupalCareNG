<?php

/**
 * @file
 * Definitions and callbacks for CARE date field.
 */

/**
 * Field instance settings form.
 */
function _care_fields_date_instance_settings_form($settings) {
  $form = [];
  $form['input_options'] = [
    '#title' => t('Input options'),
    '#type' => 'fieldset',
    '#tree' => TRUE,
  ];
  $form['input_options']['default_from_current_user'] = [
    '#title' => t("Get default value(s) as current user's date of birth, if possible."),
    '#type' => 'checkbox',
    '#default_value' => $settings['input_options']['default_from_current_user'],
  ];
  $form['input_options']['readonly_if_known'] = [
    '#title' => t('Read-only if known'),
    '#description' => t('If the field has data, make it read-only. Otherwise display form fields.'),
    '#type' => 'radios',
    '#options' => [
      'yes' => t('Yes, display read-only if user is logged in and the field is not empty.'),
      'no' => t('No, always display editable fields.'),
    ],
    '#default_value' => $settings['input_options']['readonly_if_known'],
  ];
  $form['input_options']['earliest_date'] = [
    '#title' => t('Earliest date'),
    '#description' => t('A date string as input for a PHP DateTime constructor (e.g. 1 Jan 1970, now, -110 years).'),
    '#type' => 'textfield',
    '#default_value' => $settings['input_options']['earliest_date'],
  ];
  $form['input_options']['latest_date'] = [
    '#title' => t('Latest date'),
    '#description' => t('A date string as input for a PHP DateTime constructor (e.g. 1 Jan 1970, now, -10 years).'),
    '#type' => 'textfield',
    '#default_value' => $settings['input_options']['latest_date'],
  ];
  $form['default_value_options'] = [
    '#type' => 'fieldset',
    '#title' => t('Default value'),
    '#tree' => TRUE,
    '#description' => t('The default value for this field, used when creating new content.'),
  ];
  $form['default_value_options']['default_date'] = [
    '#title' => t('Default date'),
    '#description' => t('A date string as input for a PHP DateTime constructor (e.g. 1 Jan 1970, now, -40 years).'),
    '#type' => 'textfield',
    '#default_value' => $settings['default_value_options']['default_date'],
  ];
  $form['default_value_options']['default_date_sets'] = [
    '#title' => t('Default date sets'),
    '#description' => t('A date string as input for a PHP DateTime constructor (e.g. 1 Jan 1970, now, -10 years).'),
    '#type' => 'radios',
    '#options' => [
      'year' => t('Year only'),
      'day' => t('Day, month, year'),
    ],
    '#default_value' => $settings['default_value_options']['default_date_sets'],
  ];
  return $form;
}

/**
 * Form definition.
 */
function _care_fields_date_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $settings = $instance['settings'];
  $default_day = '';
  $default_month = '';
  $default_year = '';
  if ($settings['input_options']['default_from_current_user']) {
    global $user;
    if ($user->uid != 0) {
      $care_user = new CareUser($user);
      $dob_field = $care_user->getDobField();
      if ($dob_field && $dob_field->value()) {
        if (is_a($dob_field, 'EntityListWrapper')) {
          $default_day = $dob_field[0]->day->value();
          $default_month = $dob_field[0]->month->value();
          $default_year = $dob_field[0]->year->value();
        }
        elseif (is_a($dob_field, 'EntityStructureWrapper')) {
          $default_day = $dob_field->day->value();
          $default_month = $dob_field->month->value();
          $default_year = $dob_field->year->value();
        }
      }
    }
  }
  if (!$default_day) {
    $default_date = new DateTime($settings['default_value_options']['default_date']);
    $default_year = $default_date->format('Y');
    if ($settings['default_value_options']['default_date_sets'] == 'day') {
      $default_day = $default_date->format('d');
      $default_month = $default_date->format('n');
    }
    else {
      $default_day = '';
      $default_month = '';
    }
  }
  $element['prefix'] = [
    '#type' => 'markup',
    '#markup' => '<div class="container-inline">',
  ];
  if ($field['cardinality'] == 1) {
    $element['prefix'] = [
      '#type' => 'markup',
      '#markup' => '<fieldset class="form-wrapper"><legend><span class="fieldset-legend">' . $instance['label'] . '</legend><div class="fieldset-wrapper container-inline">',
    ];
  }
  $earliest_date = new DateTime($settings['input_options']['earliest_date']);
  $latest_date = new DateTime($settings['input_options']['latest_date']);
  if ($earliest_date) {
    $start_year = $earliest_date->format('Y');
  }
  else {
    $start_year = date('Y') - 5;
  }
  if ($latest_date) {
    $end_year = $latest_date->format('Y');
  }
  else {
    $end_year = date('Y') + 5;
  }
  for ($year = $start_year; $year < $end_year + 1; $year++) {
    $year_options[$year] = $year;
  }
  $year_default = isset($items[$delta]['year']) ? $items[$delta]['year'] : $default_year;
  // Add option for existing year if out of range.
  if ($year_default) {
    if ($year_default < $start_year or $year_default > $end_year) {
      $year_options[$year_default] = $year_default;
    }
  }
  $day_default = isset($items[$delta]['day']) ? $items[$delta]['day'] : $default_day;
  $month_default = isset($items[$delta]['month']) ? $items[$delta]['month'] : $default_month;
  $disabled = (($settings['input_options']['readonly_if_known'] == 'yes') && user_is_logged_in() && $day_default && $month_default && $year_default);

  if (!$element['#required']) {
    $year_options = [
        '' => '',
      ] + $year_options;
    $default_year = '';
  }
  $element['day'] = [
    '#type' => 'select',
    '#options' => drupal_map_assoc(date_days($element['#required'])),
    '#disabled' => $disabled,
    '#default_value' => $day_default,
    '#required' => $element['#required'],
  ];
  $element['month'] = [
    '#type' => 'select',
    '#options' => date_month_names($element['#required']),
    '#disabled' => $disabled,
    '#default_value' => $month_default,
    '#required' => $element['#required'],
  ];
  $element['year'] = [
    '#type' => 'select',
    '#options' => $year_options,
    '#disabled' => $disabled,
    '#default_value' => $year_default,
    '#required' => $element['#required'],
  ];
  if ($field['cardinality'] == 1) {
    $element['suffix'] = [
      '#type' => 'markup',
      '#markup' => '</div>',
    ];
  }
  $element['local_unsent_changes'] = [
    '#type' => 'value',
    '#value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] : CARE_FIELDS_UNCHANGED,
  ];
  $element['#element_validate'] = [
    '_care_fields_field_widget_date_validate',
  ];

}

/**
 * Form validation.
 */
function _care_fields_field_widget_date_validate(&$element, &$form_state) {
  $field_names = [
    'day',
    'month',
    'year',
  ];
  foreach ($field_names as $field_name) {
    $field = $element[$field_name];
    // Set local_unsent_changes if #value not same as #default_value.
    if ($field['#value'] != $field['#default_value']) {
      form_set_value($element['local_unsent_changes'], CARE_FIELDS_CHANGED, $form_state);
    }
    // Trim all leading and trailing spaces.
    $element[$field_name]['#value'] = trim($field['#value']);
    // Set up array to be suitable for care_fields_field_is_empty.
    $element_values[$field_name] = $field['#value'];
  }
  if (isset($element['delete']) && $element['delete']['#value']) {
    form_set_value($element['local_unsent_changes'], CARE_FIELDS_DELETED, $form_state);
  }
  // Validate date parts only if field is not empty.
  if (!care_fields_field_is_empty($element_values, [
    'type' => 'care_date',
  ])) {
    // Validate.
    if (checkdate($element['month']['#value'], $element['day']['#value'], $element['year']['#value'])) {
      $field_info_instance = field_info_instance($element['#entity_type'], $element['#field_name'], $element['#bundle']);
      $settings = $field_info_instance['settings']['input_options'];
      $earliest_date = new DateTime($settings['earliest_date']);
      $latest_date = new DateTime($settings['latest_date']);
      $entered_date = DateTime::createFromFormat('Y-m-d', $element['year']['#value'] . '-' . $element['month']['#value'] . '-' . $element['day']['#value']);
      if ($entered_date < $earliest_date) {
        form_error($element, t('Date too early for %name, cannot be earlier than %date.', [
          '%name' => $element['#title'],
          '%date' => $earliest_date->format('d F Y'),
        ]));
      }
      elseif ($entered_date > $latest_date) {
        form_error($element, t('Date too late for %name, cannot be later than %date.', [
          '%name' => $element['#title'],
          '%date' => $latest_date->format('d F Y'),
        ]));
      }
    }
    else {
      form_error($element['day'], t('Invalid date for %name.', [
        '%name' => $element['#title'],
      ]));
    }
  }
}

/**
 * Field is empty test.
 */
function _care_fields_date_field_is_empty($item, $field) {
  return ($item['day'] == '');
}

/**
 * Database schema.
 */
function _care_fields_care_date_schema() {
  $schema['columns']['day'] = [
    'type' => 'int',
    'size' => 'tiny',
    'default' => 0,
    'not null' => TRUE,
  ];
  $schema['columns']['month'] = [
    'type' => 'int',
    'size' => 'tiny',
    'default' => 0,
    'not null' => TRUE,
  ];
  $schema['columns']['year'] = [
    'type' => 'int',
    'size' => 'small',
    'default' => 0,
    'not null' => TRUE,
  ];
  $schema['columns']['local_unsent_changes'] = [
    'type' => 'int',
    'size' => 'tiny',
    'default' => CARE_FIELDS_UNCHANGED,
    'not null' => TRUE,
  ];
  return $schema;
}

/**
 * Entity integration.
 */
function care_fields_date_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['day'] = [
    'type' => 'text',
    'label' => t('Day'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['month'] = [
    'type' => 'text',
    'label' => t('Month'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['year'] = [
    'type' => 'text',
    'label' => t('Year'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['local_unsent_changes'] = [
    'type' => 'integer',
    'label' => t('Local unsent changes'),
    'description' => t('0 = unchanged, 1 = pending changes, 2 = pending deletion'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
}
