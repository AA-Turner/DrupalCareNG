<?php

/**
 * @file
 * Functions to define a CARE name field.
 */

/**
 * Field instance settings form.
 */
function _care_fields_name_instance_settings_form($settings) {
  $form = [];
  $form['input_options'] = [
    '#title' => t('Input options'),
    '#type' => 'fieldset',
    '#tree' => TRUE,
  ];
  $form['input_options']['title_optional'] = [
    '#title' => t('Make the Title field optional.'),
    '#type' => 'checkbox',
    '#default_value' => $settings['input_options']['title_optional'],
  ];
  $form['input_options']['default_from_current_user'] = [
    '#title' => t('Get default value(s) from the current user, if possible.'),
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
  return $form;
}

/**
 * Form definition.
 */
function _care_fields_name_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $input_options = $instance['settings']['input_options'];
  $title_default = '';
  $forenames_default = '';
  $surname_default = '';
  if ($input_options['default_from_current_user']) {
    global $user;
    if ($user->uid != 0) {
      $care_user = new CareUser($user);
      $name_field = $care_user->getNameField();
      if ($name_field && $name_field->value()) {
        if (is_a($name_field, 'EntityListWrapper')) {
          if (isset($name_field[$delta])) {
            $title_default = $name_field[$delta]->title->value();
            $forenames_default = $name_field[$delta]->forenames->value();
            $surname_default = $name_field[$delta]->surname->value();
          }
        }
        elseif (is_a($name_field, 'EntityStructureWrapper')) {
          $title_default = $name_field->title->value();
          $forenames_default = $name_field->forenames->value();
          $surname_default = $name_field->surname->value();
        }
      }
    }
  }
  // Override defaults with any previously-entered data.
  $title_default = isset($items[$delta]['title']) ? $items[$delta]['title'] : $title_default;
  $forenames_default = isset($items[$delta]['forenames']) ? $items[$delta]['forenames'] : $forenames_default;
  $surname_default = isset($items[$delta]['surname']) ? $items[$delta]['surname'] : $surname_default;
  // Title options.
  $titles = variable_get('care_fields_titles', "Mr\nMrs\nMiss\nMs\nDr");
  $titles = preg_split('/\r\n|[\r\n]/', $titles);
  $title_options = [];
  if ($title_default == '') {
    $title_options += [
      '' => t(' - select - '),
    ];
  }
  elseif (!in_array($title_default, $titles)) {
    $title_options += [
      $title_default => $title_default,
    ];
  }
  $title_options += drupal_map_assoc($titles);
  $disabled = (($input_options['readonly_if_known'] == 'yes') && user_is_logged_in() && $title_default && $forenames_default && $surname_default);
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
  $element['title'] = [
    '#type' => 'select',
    '#title' => t('Title'),
    '#disabled' => $disabled,
    '#default_value' => $title_default,
    '#required' => ($element['#required'] && !$input_options['title_optional']),
    '#options' => $title_options,
  ];
  $element['forenames'] = [
    '#type' => 'textfield',
    '#title' => t('Forename'),
    '#disabled' => $disabled,
    '#default_value' => $forenames_default,
    '#required' => $element['#required'],
    '#size' => 30,
    '#maxlength' => 60,
  ];
  $element['surname'] = [
    '#type' => 'textfield',
    '#title' => t('Surname'),
    '#disabled' => $disabled,
    '#default_value' => $surname_default,
    '#required' => $element['#required'],
    '#size' => 30,
    '#maxlength' => 50,
  ];
  $element['suffix'] = [
    '#type' => 'markup',
    '#markup' => '</div>',
  ];
  $element['local_unsent_changes'] = [
    '#type' => 'value',
    '#value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] : CARE_FIELDS_UNCHANGED,
  ];
  $element['#element_validate'] = [
    '_care_fields_field_widget_name_validate',
  ];
}

/**
 * Form validation.
 */
function _care_fields_field_widget_name_validate(&$element, &$form_state) {
  $field_names = [
    'title',
    'forenames',
    'surname',
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
  if (isset($element['delete']) && $element['delete']['#value']) {
    form_set_value($element['local_unsent_changes'], CARE_FIELDS_DELETED, $form_state);
  }
}

/**
 * Field-is-empty test.
 */
function _care_fields_name_field_is_empty($item, $field) {
  return ($item['forenames'] == '' && $item['surname'] == '');
}

/**
 * Field database schema.
 */
function _care_fields_care_name_schema() {
  $schema['columns']['title'] = [
    'type' => 'varchar',
    'length' => 40,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['forenames'] = [
    'type' => 'varchar',
    'length' => 60,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['surname'] = [
    'type' => 'varchar',
    'length' => 50,
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
function care_fields_name_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['title'] = [
    'type' => 'text',
    'label' => t('Title'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['forenames'] = [
    'type' => 'text',
    'label' => t('Fornames'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['surname'] = [
    'type' => 'text',
    'label' => t('Surname'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['local_unsent_changes'] = [
    'type' => 'integer',
    'label' => t('Local unsent changes'),
    'description' => t('0 = unchanged, 1 = pending changes, 2 = pending deletion'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
}
