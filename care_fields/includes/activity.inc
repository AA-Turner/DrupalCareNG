<?php

/**
 * @file
 * Definitions and callbacks for CARE activity field.
 */

/**
 * Field instance settings form.
 */
function _care_fields_activity_instance_settings_form($settings) {
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
  $vocabularies = taxonomy_get_vocabularies();
  $vocabulary_options = [];
  // ToDO: Add vocab descr.
  foreach ($vocabularies as $vid => $vocabulary) {
    if (substr($vocabulary->machine_name, 0, 22) == 'care_taxonomy_activity') {
      $vocabulary_options[$vid] = $vocabulary->name;
    }
  }
  $form['input_options']['activity_type'] = [
    '#title' => t('Activity type for this field.'),
    '#type' => 'select',
    '#options' => $vocabulary_options,
    '#default_value' => $settings['input_options']['activity_type'],
  ];
  return $form;
}

/**
 * Form definition.
 *
 * Very like options_field_widget_form from options.module.
 *
 * @noinspection PhpUnused
 */
function _care_fields_activity_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $value_key = key($field['columns']);

  $type = 'buttons';
  $multiple = $field['cardinality'] > 1 || $field['cardinality'] == FIELD_CARDINALITY_UNLIMITED;
  $required = $element['#required'];
  $has_value = isset($items[0][$value_key]);

  // Set widget rendering properties.
  // $properties = _options_properties($type, $multiple, $required, $has_value);
  $properties = [
    'filter_xss' => TRUE,
    'strip_tags' => FALSE,
    'strip_tags_and_unescape' => FALSE,
    'empty_option' => FALSE,
    'optgroups' => FALSE,
  ];

  // Prepare the list of options.
  $options = [];
  $vocabulary_id = $instance['settings']['input_options']['activity_type'];
  $terms = taxonomy_term_load_multiple([], ['vid' => $vocabulary_id]);
  foreach ($terms as $term) {
    $options[$term->tid] = $term->description . ' (' . $term->name . ')';
  }
  // Put current field values in shape.
  $default_value = _options_storage_to_form($items, $options, $value_key, $properties);

  // If required and there is one single option, preselect it.
  if ($required && count($options) == 1) {
    reset($options);
    $default_value = [key($options)];
  }

  // If this is a single-value field, take the first default value, or
  // default to NULL so that the form element is properly recognized as
  // not having a default value.
  if (!$multiple) {
    $default_value = $default_value ? reset($default_value) : NULL;
  }

  $element['activity'] = [
    '#type' => $multiple ? 'checkboxes' : 'radios',
    '#title' => $element['#title'],
    '#description' => $element['#description'],
    '#required' => $element['#required'],
    // Radio buttons need a scalar value.
    '#default_value' => $default_value,
    '#options' => $options,
  ];

  $element += [
    '#value_key' => $value_key,
    '#element_validate' => ['care_fields_field_widget_activity_validate'],
    '#properties' => $properties,
  ];

  $element['local_unsent_changes'] = [
    '#type' => 'value',
    '#value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] : CARE_FIELDS_UNCHANGED,
  ];

  return $element;
}

/**
 * Form validation.
 *
 * @noinspection PhpUnused
 */
function care_fields_field_widget_activity_validate(&$element, &$form_state) {
  if ($element['#required'] && $element['#value'] == '_none') {
    form_error($element, t('!name field is required.', ['!name' => $element['#title']]));
  }
  // Set local_unsent_changes if #value not same as #default_value.
  if ($element['activity']['#value'] != $element['activity']['#default_value']) {
    form_set_value($element['local_unsent_changes'], CARE_FIELDS_CHANGED, $form_state);
  }
  // Transpose selections from field => delta to delta => field, turning
  // multiple selected options into multiple parent elements.
  $items = options_array_transpose(['activity' => array_values((array) $element['activity']['#value'])]);
  foreach ($items as $index => $item) {
    $items[$index] += ['local_unsent_changes' => CARE_FIELDS_CHANGED];
  }
  form_set_value($element, $items, $form_state);
}

/**
 * Field-is-empty test.
 */
function _care_fields_activity_field_is_empty($item, $field) {
  return FALSE;
}

/**
 * Display formatter.
 */
function _care_fields_care_activity_formatter($delta, $item, $instance) {
  $element = [];
  $element[$delta]['#markup'] = check_plain($item['activity']);
  return $element;
}

/**
 * Field database schema.
 */
function _care_fields_care_activity_schema() {
  $schema['columns']['activity'] = [
    'type' => 'varchar',
    'length' => 255,
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
function care_fields_activity_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['activity'] = [
    'type' => 'text',
    'label' => t('activity'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['local_unsent_changes'] = [
    'type' => 'integer',
    'label' => t('Local unsent changes'),
    'description' => t('0 = unchanged, 1 = pending changes, 2 = pending deletion'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
}
