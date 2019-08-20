<?php

/**
 * @file
 * Functions to define a CARE membership number field.
 */

/**
 * Field instance settings form.
 */
function _care_fields_membership_number_instance_settings_form($settings) {
  $form = array();
  $form['input_options'] = array(
    '#title' => t('Input options'),
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );
  $form['input_options']['default_from_current_user'] = array(
    '#title' => t('Get default value from the current user, if possible.'),
    '#type' => 'checkbox',
    '#default_value' => $settings['input_options']['default_from_current_user'],
  );
  $form['input_options']['readonly_if_known'] = array(
    '#title' => t('Read-only if known'),
    '#description' => t('If the field has data, make it read-only. Otherwise display form fields.'),
    '#type' => 'radios',
    '#options' => array(
      'yes' => t('Yes, display read-only if user is logged in and the field is not empty.'),
      'no' => t('No, always display editable fields.'),
    ),
    '#default_value' => $settings['input_options']['readonly_if_known'],
  );
  return $form;
}

/**
 * Form definition.
 */
function _care_fields_membership_number_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $input_options = $instance['settings']['input_options'];
  $input_options += array(
    'default_from_current_user' => FALSE,
    'readonly_if_known' => 'no',
  );
  $number_default = '';
  global $user;
  if ($input_options['default_from_current_user']) {
    if ($user->uid != 0) {
      $care_user = new CareUser($user);
      $membership_number = $care_user->getMembershipNumber();
      if ($membership_number) {
        $number_default = $membership_number;
      }
    }
  }
  // Override defaults with any previously-entered data.
  $number_default = isset($items[$delta]['number']) ? $items[$delta]['number'] : $number_default;
  // Check to see if $user has the staff member role.
  $shop_admin = (user_access('administer users')) ? 'TRUE' : 'FALSE' ;
  $disabled = (($input_options['readonly_if_known'] == 'yes') and user_is_logged_in() and $number_default and ($shop_admin == 'FALSE'));
  $element['number'] = array(
    '#type' => 'textfield',
    '#title' => $element['#title'],
    '#description' => $element['#description'],
    '#default_value' => $number_default,
    '#required' => $element['#required'],
    '#disabled' => $disabled,
    '#size' => 30,
    '#maxlength' => 30,
  );
  $element['local_unsent_changes'] = array(
    '#type' => 'value',
    '#value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] : CARE_FIELDS_UNCHANGED,
  );
  $element['#element_validate'] = array(
    '_care_fields_field_membership_number_validate',
  );
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
    'not null' => FALSE,
  );
  $schema['columns']['local_unsent_changes'] = array(
    'type' => 'int',
    'size' => 'tiny',
    'default' => CARE_FIELDS_UNCHANGED,
    'not null' => TRUE,
  );
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
    'setter callback' => 'entity_property_verbatim_set',
  );
  $property['property info']['local_unsent_changes'] = array(
    'type' => 'integer',
    'label' => t('Local unsent changes'),
    'description' => t('0 = unchanged, 1 = pending changes, 2 = pending deletion'),
    'setter callback' => 'entity_property_verbatim_set',
  );
}

