<?php

/**
 * @file
 * Functions to define a CARE telephone field.
 */

/**
 * Field instance settings form.
 */
function _care_fields_telephone_instance_settings_form($settings) {
  $settings['input_options'] += [
    'default_from_current_user' => 0,
    'display_type_field' => 1,
  ];
  $form = [];
  $form['input_options'] = [
    '#title' => t('Input options'),
    '#type' => 'fieldset',
    '#tree' => TRUE,
  ];
  $form['input_options']['default_from_current_user'] = [
    '#title' => t('Get default value(s) from the current user, if possible.'),
    '#type' => 'checkbox',
    '#default_value' => $settings['input_options']['default_from_current_user'],
  ];
  $form['input_options']['display_type_field'] = [
    '#title' => t('Telephone types'),
    '#type' => 'radios',
    '#options' => [
      1 => 'Allow user to select telephone types',
      0 => t('Fix telephone type for this field'),
    ],
    '#default_value' => $settings['input_options']['display_type_field'],
  ];
  $telephone_devices = preg_split('/\r\n|[\r\n]/', variable_get('care_fields_telephone_devices', "TH|Home telephone\nTE|Work Telephone"));
  $telephone_device_options = [];
  foreach ($telephone_devices as $index => $telephone_device) {
    if (trim($telephone_device)) {
      $data = explode("|", $telephone_device);
      $telephone_device_options[$data[0]] = $data[1];
    }
  }
  $form['input_options']['fixed_type'] = [
    '#title' => t('Use this telephone type for all numbers in this field'),
    '#type' => 'select',
    '#options' => $telephone_device_options,
    '#states' => [
      'visible' => [
        ':input[name="instance[settings][input_options][display_type_field]"]' => [
          'value' => 0,
        ],
      ],
    ],
    '#default_value' => $settings['input_options']['fixed_type'],
  ];

  return $form;
}

/**
 * Form definition.
 */
function _care_fields_telephone_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $settings = $instance['settings']['input_options'];
  $telephone_default = '';
  $device_code_default = '';
  if ($settings['default_from_current_user']) {
    global $user;
    if ($user->uid != 0) {
      $care_user = new CareUser($user);
      $telephone_field = $care_user->getTelephoneField();
      if ($telephone_field and $telephone_field->value()) {
        if (is_a($telephone_field, 'EntityListWrapper')) {
          if (isset($telephone_field[$delta])) {
            $telephone_default = $telephone_field[$delta]->telephone->value();
            $device_code_default = $telephone_field[$delta]->device_code->value();
          }
        }
        elseif (is_a($telephone_field, 'EntityStructureWrapper')) {
          $telephone_default = $telephone_field->telephone->value();
          $device_code_default = $telephone_field->device_code->value();
        }
      }
    }
  }

  $element['communication_number'] = [
    '#type' => 'value',
    '#value' => isset($items[$delta]['communication_number']) ? $items[$delta]['communication_number'] : 0,
  ];
  if ($field['cardinality'] == 1) {
    $element['prefix'] = [
      '#type' => 'markup',
      '#markup' => '<fieldset class="form-wrapper"><legend><span class="fieldset-legend">' . $instance['label'] . '</legend><div class="fieldset-wrapper">',
    ];
  }
  $element['inline_start'] = [
    '#markup' => '<div class="fieldset-wrapper container-inline">',
  ];
  $element['telephone'] = [
    '#type' => 'textfield',
    '#title' => t('Telephone'),
    '#default_value' => isset($items[$delta]['telephone']) ? $items[$delta]['telephone'] : $telephone_default,
    '#required' => $element['#required'],
    '#maxlength' => 20,
    '#size' => 30,
  ];
  if ($settings['display_type_field']) {
    $telephone_device_options = [];
    $telephone_devices = preg_split('/\r\n|[\r\n]/', variable_get('care_fields_telephone_devices', "TH|Home telephone\nTE|Work Telephone"));
    foreach ($telephone_devices as $index => $telephone_device) {
      if (trim($telephone_device)) {
        $data = explode("|", $telephone_device);
        $telephone_device_options[$data[0]] = $data[1];
      }
    }
    if (!$device_code_default) {
      $device_code_default = key($telephone_device_options);
    }
    $telephone_device_default = isset($items[$delta]['device_code']) ? $items[$delta]['device_code'] : $device_code_default;
    if (!array_key_exists($telephone_device_default, $telephone_device_options)) {
      $telephone_device_options += [
        $telephone_device_default => 'Device code ' . $telephone_device_default,
      ];
    }
    $element['device_code'] = [
      '#type' => 'select',
      '#title' => t('Type'),
      '#options' => $telephone_device_options,
      '#default_value' => $telephone_device_default,
      '#required' => $element['#required'],
    ];
  }
  else {
    $element['device_code'] = [
      '#type' => 'value',
      '#value' => $settings['fixed_type'],
      '#default_value' => $settings['fixed_type'],
    ];
  }
  if ($field['cardinality'] != 1) {
    $element['delete'] = [
      '#type' => 'checkbox',
      '#title' => t('Delete this telephone number'),
      '#default_value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] == CARE_FIELDS_DELETED : FALSE,
    ];
  }
  if (variable_get('care_fields_show_numbers', TRUE) and user_access('care_fields view CARE numbers')) {
    $element['communication_number_display'] = [
      '#type' => 'markup',
      '#prefix' => '<div class="carenumber">',
      '#markup' => t('CARE communication number: !number', [
        '!number' => isset($items[$delta]['communication_number']) ? (int) $items[$delta]['communication_number'] : '<i>none</i>',
      ]),
      '#suffix' => '</div>',
    ];
  }
  $element['inline_end'] = [
    '#markup' => '</div>',
  ];
  if ($field['cardinality'] == 1) {
    $element['suffix'] = [
      '#type' => 'markup',
      '#markup' => '</fieldset>',
    ];
  }
  $element['local_unsent_changes'] = [
    '#type' => 'value',
    '#value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] : CARE_FIELDS_UNCHANGED,
  ];
  $element['#element_validate'] = [
    '_care_fields_field_widget_telephone_validate',
  ];
}

/**
 * Form validation.
 */
function _care_fields_field_widget_telephone_validate(&$element, &$form_state) {
  $element_values = [
    'communication_number' => $element['communication_number']['#value'],
  ];
  $field_names = [
    'telephone',
    'device_code',
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
  if (isset($element['delete']) and $element['delete']['#value']) {
    form_set_value($element['local_unsent_changes'], CARE_FIELDS_DELETED, $form_state);
  }
  // Validate address parts only if field is not empty.
  if (!care_fields_field_is_empty($element_values, [
    'type' => 'care_telephone',
  ])) {
    // Validate.
    if (!isset($element['delete']) or !$element['delete']['#value']) {
      if ($element['telephone']['#value'] == '') {
        form_error($element['telephone'], t('Telephone number is required for %name.', [
          '%name' => $element['#title'],
        ]));
      }
      elseif (strlen($element['telephone']['#value']) > 20) {
        form_error($element['telephone'], t('Telephone numbers cannot be longer than 20 characters.', [
          '%name' => $element['#title'],
        ]));
      }
    }
  }
}

/**
 * Field-is-empty test.
 */
function _care_fields_telephone_field_is_empty($item, $field) {
  $not_in_care = ($item['communication_number'] == 0);
  $empty_telephone = ($item['telephone'] == '');
  $delete = (isset($item['delete']) and $item['delete']);
  return ($not_in_care && ($delete || $empty_telephone));
}

/**
 * Display formatter.
 */
function _care_fields_care_telephone_formatter($delta, $item, $instance) {
  $element = [];
  $element[$delta]['#markup'] = check_plain($item['telephone']);
  if ($instance['settings']['input_options']['display_type_field']) {
    $device_options = [];
    $device_codes = preg_split('/\r\n|[\r\n]/', variable_get('care_fields_telephone_devices'));
    foreach ($device_codes as $device_code) {
      if ($device_code) {
        $data = explode('|', $device_code);
        $device_options[$data[0]] = $data[1];
      }
    }
    if (array_key_exists($item['device_code'], $device_options)) {
      $device = $device_options[$item['device_code']];
    }
    else {
      $device = 'Device code ' . $item['device_code'];
    }
    $element[$delta]['#markup'] .= ' (' . check_plain($device) . ')';
  }
  if (variable_get('care_fields_show_numbers', TRUE) and user_access('care_fields view CARE numbers')) {
    $element[$delta]['#markup'] .= ' <i>[' . $item['communication_number'];
    if ($item['device_default']) {
      $element[$delta]['#markup'] .= ', DeviceDefault';
    }
    $element[$delta]['#markup'] .= ']</i>';
  }
  return $element;
}

/**
 * Database schema.
 */
function _care_fields_care_telephone_schema() {
  $schema['columns']['communication_number'] = [
    'type' => 'int',
    'size' => 'normal',
    'default' => 0,
    'not null' => TRUE,
  ];
  $schema['columns']['telephone'] = [
    'type' => 'varchar',
    'length' => 20,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['device_code'] = [
    'type' => 'varchar',
    'length' => 8,
    'default' => '',
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
function care_fields_telephone_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['communication_number'] = [
    'type' => 'text',
    'label' => t('Communication number'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['telephone'] = [
    'type' => 'text',
    'label' => t('Telephone'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['device_code'] = [
    'type' => 'text',
    'label' => t('Type'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['local_unsent_changes'] = [
    'type' => 'integer',
    'label' => t('Local unsent changes'),
    'description' => t('0 = unchanged, 1 = pending changes, 2 = pending deletion'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
}
