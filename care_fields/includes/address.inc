<?php

/**
 * @file
 * Definitions and callbacks for CARE address field.
 */

/**
 * Field instance settings form.
 */
function _care_fields_address_instance_settings_form($settings) {
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
  return $form;
}

/**
 * Form definition.
 */
function _care_fields_address_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $countries = variable_get('care_fields_countries', 'UK|United Kingdom');
  $countries = preg_split('/\r\n|[\r\n]/', $countries);
  $country_default = isset($items[$delta]['country_code']) ? $items[$delta]['country_code'] : variable_get('care_fields_country_default', 'UK');
  $country_options = [];
  if ($country_default == '') {
    $country_options += [
      '' => t(' - select - '),
    ];
  }
  foreach ($countries as $index => $country) {
    if (trim($country)) {
      $data = explode("|", $country);
      $country_options[$data[0]] = $data[1];
    }
  }
  $element['address_number'] = [
    '#type' => 'value',
    '#value' => isset($items[$delta]['address_number']) ? $items[$delta]['address_number'] : 0,
  ];
  if ($field['cardinality'] == 1) {
    $element['prefix'] = [
      '#type' => 'markup',
      '#markup' => '<fieldset class="form-wrapper"><legend><span class="fieldset-legend">' . $instance['label'] . '</legend><div class="fieldset-wrapper">',
    ];
  }
  if (variable_get('care_fields_show_numbers', TRUE) && user_access('care_fields view CARE numbers')) {
    $element['address_number_display'] = [
      '#type' => 'markup',
      '#prefix' => '<div class="carenumber">',
      '#markup' => t('CARE address number: !number', [
        '!number' => isset($items[$delta]['address_number']) ? (int) $items[$delta]['address_number'] : '<i>none</i>',
      ]),
      '#suffix' => '</div>',
    ];
  }
  $element['address_line_1'] = [
    '#type' => 'textfield',
    '#title' => t('Address line 1'),
    '#default_value' => isset($items[$delta]['address_line_1']) ? $items[$delta]['address_line_1'] : '',
    '#required' => $element['#required'],
    '#size' => 30,
  ];
  $element['address_line_2'] = [
    '#type' => 'textfield',
    '#title' => t('Address line 2'),
    '#default_value' => isset($items[$delta]['address_line_2']) ? $items[$delta]['address_line_2'] : '',
    '#size' => 30,
  ];
  $element['address_line_3'] = [
    '#type' => 'textfield',
    '#title' => t('Address line 3'),
    '#default_value' => isset($items[$delta]['address_line_3']) ? $items[$delta]['address_line_3'] : '',
    '#size' => 30,
  ];
  $element['town'] = [
    '#type' => 'textfield',
    '#title' => t('Town'),
    '#default_value' => isset($items[$delta]['town']) ? $items[$delta]['town'] : '',
    '#required' => $element['#required'],
    '#size' => 30,
  ];
  $element['county'] = [
    '#type' => 'textfield',
    '#title' => t('County'),
    '#default_value' => isset($items[$delta]['county']) ? $items[$delta]['county'] : '',
    '#size' => 30,
  ];
  $element['country_code'] = [
    '#type' => 'select',
    '#title' => t('Country'),
    '#options' => $country_options,
    '#default_value' => $country_default,
    '#required' => $element['#required'],
  ];
  $element['postcode'] = [
    '#type' => 'textfield',
    '#title' => t('Postcode'),
    '#default_value' => isset($items[$delta]['postcode']) ? $items[$delta]['postcode'] : '',
    '#required' => $element['#required'],
    '#size' => 10,
    '#maxlength' => 10,
  ];

  $element['address_index'] = [
    '#type' => 'value',
    '#value' => 0,
  ];
  if ($field['cardinality'] != 1) {
    $element['delete'] = [
      '#type' => 'checkbox',
      '#title' => t('Delete this address.'),
      '#default_value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] == CARE_FIELDS_DELETED : FALSE,
    ];
  }
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
    '_care_fields_field_widget_address_validate',
  ];
}

/**
 * Form validation.
 */
function _care_fields_field_widget_address_validate(&$element, &$form_state) {
  $element_values = [
    'address_number' => $element['address_number']['#value'],
  ];
  $field_names = [
    'address_line_1',
    'address_line_2',
    'address_line_3',
    'town',
    'county',
    'country_code',
    'postcode',
  ];
  foreach ($field_names as $field_name) {
    $field = $element[$field_name];
    // Set local_unsent_changes if #value not same as #default_value.
    // TODO: store value last read from CARE and use that?
    if ($field['#value'] != $field['#default_value']) {
      form_set_value($element['local_unsent_changes'], CARE_FIELDS_CHANGED, $form_state);
    }
    // Trim all leading and trailing spaces.
    $element[$field_name]['#value'] = trim($field['#value']);
    // Set up array to be suitable for care_fields_field_is_empty.
    $element_values[$field_name] = $field['#value'];
  }
  if (isset($element['delete']) && isset($element['delete']['#value']) && $element['delete']['#value']) {
    form_set_value($element['local_unsent_changes'], CARE_FIELDS_DELETED, $form_state);
  }
  // Validate address parts only if field is not empty.
  if (!care_fields_field_is_empty($element_values, [
    'type' => 'care_address',
  ])) {
    // Validate.
    if (!isset($element['delete']) || !isset($element['delete']['#value']) || !$element['delete']['#value']) {
      if ($element['address_line_1']['#value'] == '') {
        form_error($element['address_line_1'], t('%field is required for %name.', [
          '%field' => t('Address line 1'),
          '%name' => $element['#title'],
        ]));
      }
      if ($element['town']['#value'] == '') {
        form_error($element['town'], t('%field is required for %name', [
          '%field' => t('Town'),
          '%name' => $element['#title'],
        ]));
      }
      if ($element['country_code']['#value'] == 'UK' && $element['postcode']['#value'] == '') {
        form_error($element['postcode'], t('Postcode is required in UK for %name.', [
          '%name' => $element['#title'],
        ]));
      }
      if (drupal_strlen($element['postcode']['#value']) > 10) {
        form_error($element['postcode'], t('Postcode for %name cannot be longer than ten characters.', [
          '%name' => $element['#title'],
        ]));
      }
    }
  }
}

/**
 * Field is empty test.
 */
function _care_fields_address_field_is_empty($item, $field) {
  $not_in_care = ($item['address_number'] == 0);
  $empty_line_1 = ($item['address_line_1'] == '');
  $empty_line_2 = ($item['address_line_2'] == '');
  $empty_line_3 = ($item['address_line_3'] == '');
  $empty_town = ($item['town'] == '');
  $empty_postcode = (trim($item['postcode']) == '');
  $delete = (isset($item['delete']) and $item['delete']);
  $empty = ($not_in_care and ($delete or ($empty_line_1 and $empty_line_2 and $empty_line_3 and $empty_town and $empty_postcode)));
  return $empty;
}

/**
 * Database schema.
 */
function _care_fields_care_address_schema() {
  $schema['columns']['address_number'] = [
    'type' => 'int',
    'size' => 'normal',
    'default' => 0,
    'not null' => TRUE,
  ];
  $schema['columns']['address_line_1'] = [
    'type' => 'varchar',
    'length' => 255,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['address_line_2'] = [
    'type' => 'varchar',
    'length' => 255,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['address_line_3'] = [
    'type' => 'varchar',
    'length' => 255,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['town'] = [
    'type' => 'varchar',
    'length' => 255,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['county'] = [
    'type' => 'varchar',
    'length' => 255,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['country_code'] = [
    'type' => 'varchar',
    'length' => 255,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['postcode'] = [
    'type' => 'varchar',
    'length' => 12,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['address_index'] = [
    'type' => 'int',
    'size' => 'normal',
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
function care_fields_address_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['address_number'] = [
    'type' => 'text',
    'label' => t('Address number'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['address_line_1'] = [
    'type' => 'text',
    'label' => t('Address line 1'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['address_line_2'] = [
    'type' => 'text',
    'label' => t('Address line 2'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['address_line_3'] = [
    'type' => 'text',
    'label' => t('Address line 3'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['town'] = [
    'type' => 'text',
    'label' => t('Town'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['county'] = [
    'type' => 'text',
    'label' => t('County'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['country_code'] = [
    'type' => 'text',
    'label' => t('Country code'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['postcode'] = [
    'type' => 'text',
    'label' => t('Postcode'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['local_unsent_changes'] = [
    'type' => 'integer',
    'label' => t('Local unsent changes'),
    'description' => t('0 = unchanged, 1 = pending changes, 2 = pending deletion'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['address_index'] = [
    'type' => 'text',
    'label' => t('Address index (from select)'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
}
