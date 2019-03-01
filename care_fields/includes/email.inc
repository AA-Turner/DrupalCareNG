<?php

/**
 * @file
 * Definitions and callbacks for CARE email field.
 */

/**
 * Field instance settings form.
 */
function _care_fields_email_instance_settings_form($settings) {
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
    '#title' => t('Display email type field to user.'),
    '#type' => 'checkbox',
    '#default_value' => $settings['input_options']['display_type_field'],
  ];
  $email_devices = preg_split('/\r\n|[\r\n]/', variable_get('care_fields_email_devices', "EM|Home email\nEW|Work email"));
  $email_device_options = [];
  foreach ($email_devices as $index => $email_device) {
    if (trim($email_device)) {
      $data = explode("|", $email_device);
      $email_device_options[$data[0]] = $data[1];
    }
  }
  $form['input_options']['fixed_type'] = [
    '#title' => t('Use this type for all email addresses.'),
    '#type' => 'select',
    '#options' => $email_device_options,
    '#states' => [
      'visible' => [
        ':input[name="instance[settings][input_options][display_type_field]"]' => [
          'checked' => FALSE,
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
function _care_fields_email_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $settings = $instance['settings']['input_options'];
  $email_default = '';
  $device_code_default = '';
  if ($settings['default_from_current_user']) {
    global $user;
    if ($user->uid != 0) {
      $care_user = new CareUser($user);
      $email_field = $care_user->getEmailField();
      if ($email_field and $email_field->value()) {
        if (is_a($email_field, 'EntityListWrapper')) {
          if (isset($email_field[$delta])) {
            $email_default = $email_field[$delta]->email->value();
            $device_code_default = $email_field[$delta]->device_code->value();
          }
        }
        elseif (is_a($email_field, 'EntityStructureWrapper')) {
          $email_default = $email_field->email->value();
          $device_code_default = $email_field->device_code->value();
        }
      }
    }
  }

  $element['communication_number'] = [
    '#type' => 'value',
    '#value' => isset($items[$delta]['communication_number']) ? $items[$delta]['communication_number'] : 0,
  ];
  $element['device_default'] = [
    '#type' => 'value',
    '#value' => isset($items[$delta]['device_default']) ? $items[$delta]['device_default'] : 0,
  ];
  if ($field['cardinality'] == 1) {
    $element['prefix'] = [
      '#type' => 'markup',
      '#markup' => '<fieldset class="form-wrapper"><legend><span class="fieldset-legend">' . $instance['label'] . '</legend>',
    ];
  }
  $element['inline_start'] = [
    '#markup' => '<div class="fieldset-wrapper container-inline">',
  ];
  $element['email'] = [
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#default_value' => isset($items[$delta]['email']) ? $items[$delta]['email'] : $email_default,
    '#required' => $element['#required'],
    '#size' => 30,
  ];
  if ($settings['display_type_field']) {
    $email_devices = preg_split('/\r\n|[\r\n]/', variable_get('care_fields_email_devices', "EM|Home email\nEW|Work email"));
    $email_device_options = [];
    foreach ($email_devices as $index => $email_device) {
      if (trim($email_device)) {
        $data = explode("|", $email_device);
        $email_device_options[$data[0]] = $data[1];
      }
    }
    if (!$device_code_default) {
      $device_code_default = key($email_device_options);
    }
    $email_device_default = isset($items[$delta]['device_code']) ? $items[$delta]['device_code'] : $device_code_default;
    if (!array_key_exists($email_device_default, $email_device_options)) {
      $email_device_options += [
        $email_device_default => 'Device code ' . $email_device_default,
      ];
    }
    $element['device_code'] = [
      '#type' => 'select',
      '#title' => t('Type'),
      '#options' => $email_device_options,
      '#default_value' => $email_device_default,
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
      '#title' => t('Delete this email address.'),
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
    '_care_fields_field_widget_email_validate',
  ];
}

/**
 * Form validation.
 */
function _care_fields_field_widget_email_validate(&$element, &$form_state) {
  $element_values = [
    'communication_number' => $element['communication_number']['#value'],
  ];
  $field_names = [
    'email',
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
    'type' => 'care_email',
  ])) {
    // Validate.
    if (!isset($element['delete']) or !$element['delete']['#value']) {
      if ($element['email']['#value'] == '' and $element['#required']) {
        form_error($element['email'], t('Email address is required for %name.', [
          '%name' => $element['#title'],
        ]));
      }
    }
  }
}

/**
 * Field-is-empty test.
 */
function _care_fields_email_field_is_empty($item, $field) {
  $not_in_care = ($item['communication_number'] == 0);
  $empty_email = ($item['email'] == '');
  $delete = (isset($item['delete']) and $item['delete']);
  return ($not_in_care && ($delete || $empty_email));
}

/**
 * Display formatter.
 */
function _care_fields_care_email_formatter($delta, $item, $instance) {
  $element = [];
  $element[$delta]['#markup'] = check_plain($item['email']);
  if ($instance['settings']['input_options']['display_type_field']) {
    $device_options = [];
    $device_codes = preg_split('/\r\n|[\r\n]/', variable_get('care_fields_email_devices'));
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
 * Field database schema.
 */
function _care_fields_care_email_schema() {
  $schema['columns']['communication_number'] = [
    'type' => 'int',
    'size' => 'normal',
    'default' => 0,
    'not null' => TRUE,
  ];
  $schema['columns']['email'] = [
    'type' => 'varchar',
    'length' => 255,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['device_code'] = [
    'type' => 'varchar',
    'length' => 8,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['device_default'] = [
    'type' => 'int',
    'size' => 'tiny',
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
function care_fields_email_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['communication_number'] = [
    'type' => 'text',
    'label' => t('Communication number'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['email'] = [
    'type' => 'text',
    'label' => t('Email'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['device_code'] = [
    'type' => 'text',
    'label' => t('Type'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['device_default'] = [
    'type' => 'integer',
    'label' => t('Device default'),
    'description' => t('0 = No, 1 = Yes'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['local_unsent_changes'] = [
    'type' => 'integer',
    'label' => t('Local unsent changes'),
    'description' => t('0 = unchanged, 1 = pending changes, 2 = pending deletion'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
}
