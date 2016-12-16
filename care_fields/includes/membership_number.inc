<?php

/**
 * @file
 * Functions to define a CARE membership number field.
 */

/**
 * Form definition.
 */
function _care_fields_membership_number_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $element['number'] = array(
    '#type' => 'textfield',
    '#title' => $element['#title'],
    '#description' => $element['#description'],
    '#default_value' => isset($items[$delta]['number']) ? $items[$delta]['number'] : '',
    '#required' => $element['#required'],
    '#size' => 30,
    '#maxlength' => 30);
  $element['local_unsent_changes'] = array(
    '#type' => 'value',
    '#value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] : 0);
  $element['#element_validate'] = array(
    '_care_fields_field_membership_number_validate');
}

/**
 * Form validation.
 */
function _care_fields_field_membership_number_validate(&$element, &$form_state) {
  // Remove all non-integer characters from entered number.
  $trimmed_number = preg_replace('/[^0-9]/', '', $element['number']['#value']);
  $element['number']['#value'] = $trimmed_number;
  form_set_value($element['number'], $trimmed_number, $form_state);
  if (strlen($trimmed_number) > 9) {
    form_error($element['number'], t('Your membership number should not be more than 9 numeric digits long.'));
  }
}

/**
 * Field-is-empty test.
 */
function _care_fields_membership_number_field_is_empty($item, $field) {
  return ($item['number'] == '');
}

/**
 * Database schema.
 */
function _care_fields_care_membership_number_schema() {
  $schema['columns']['number'] = array(
    'type' => 'int',
    'size' => 'normal',
    'default' => 0,
    'not null' => FALSE);
  $schema['columns']['local_unsent_changes'] = array(
    'type' => 'int',
    'size' => 'tiny',
    'default' => 0,
    'not null' => TRUE);
  return $schema;
}

/**
 * Entity integration.
 */
function care_fields_membership_number_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['number'] = array(
    'type' => 'text',
    'label' => t('Membership Number'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['local_unsent_changes'] = array(
    'type' => 'boolean',
    'label' => t('Local unsent changes'),
    'setter callback' => 'entity_property_verbatim_set');
}
