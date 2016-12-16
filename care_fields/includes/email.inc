<?php

/**
 * @file
 * Definitions and callbacks for CARE email field.
 */

/**
 * Form definition.
 */
function _care_fields_email_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $element['communication_number'] = array(
    '#type' => 'value',
    '#value' => isset($items[$delta]['communication_number']) ? $items[$delta]['communication_number'] : 0);
  $element['device_default'] = array(
    '#type' => 'value',
    '#value' => isset($items[$delta]['device_default']) ? $items[$delta]['device_default'] : 0);
  if ($field['cardinality'] == 1) {
    $element['prefix'] = array(
      '#type' => 'markup',
      '#markup' => '<fieldset class="form-wrapper"><legend><span class="fieldset-legend">' . $instance['label'] . '</legend>');
  }
  $element['inline_start'] = array(
    '#markup' => '<div class="fieldset-wrapper container-inline">');
  $element['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#default_value' => isset($items[$delta]['email']) ? $items[$delta]['email'] : '',
    '#required' => $element['#required'],
    '#size' => 30);
  $email_devices = preg_split('/\r\n|[\r\n]/', variable_get('care_fields_email_devices', "EM|Home email\nEW|Work email"));
  $email_device_options = array();
  foreach ($email_devices as $index => $email_device) {
    if (trim($email_device)) {
      $data = explode("|", $email_device);
      $email_device_options[$data[0]] = $data[1];
    }
  }
  $email_device_default = isset($items[$delta]['device_code']) ? $items[$delta]['device_code'] : key($email_device_options);
  if (!in_array($email_device_default, array_keys($email_device_options))) {
    $email_device_options += array(
      $email_device_default => 'Device code ' . $email_device_default);
  }
  $element['device_code'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#options' => $email_device_options,
    '#default_value' => $email_device_default,
    '#required' => $element['#required']);
  if ($delta > 0) {
    $element['delete'] = array(
      '#type' => 'checkbox',
      '#title' => t('Delete this email address.'),
      '#default_value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] == 2 : FALSE);
  }
  if (variable_get('care_fields_show_numbers', TRUE) and user_access('care_fields view CARE numbers')) {
    $element['communication_number_display'] = array(
      '#type' => 'markup',
      '#prefix' => '<div class="carenumber">',
      '#markup' => t('CARE communication number: !number', array(
        '!number' => isset($items[$delta]['communication_number']) ? (int)$items[$delta]['communication_number'] : '<i>none</i>')),
      '#suffix' => '</div>');
  }
  $element['inline_end'] = array(
    '#markup' => '</div>');
  if ($field['cardinality'] == 1) {
    $element['suffix'] = array(
      '#type' => 'markup',
      '#markup' => '</fieldset>');
  }
  $element['local_unsent_changes'] = array(
    '#type' => 'value',
    '#value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] : 0);
  $element['#element_validate'] = array(
    '_care_fields_field_widget_email_validate');
}

/**
 * Form validation.
 */
function _care_fields_field_widget_email_validate(&$element, &$form_state) {
  $element_values = array(
    'communication_number' => $element['communication_number']['#value']);
  $field_names = array(
    'email',
    'device_code');
  foreach ($field_names as $field_name) {
    $field = $element[$field_name];
    // Set local_unsent_changes if #value not same as #default_value.
    // TODO: store value last read from CARE and use that?
    if ($field['#value'] != $field['#default_value']) {
      form_set_value($element['local_unsent_changes'], 1, $form_state);
    }
    // Trim all leading and trailing spaces.
    $element[$field_name]['#value'] = trim($field['#value']);
    // Set up array to be suitable for care_fields_field_is_empty.
    $element_values[$field_name] = $field['#value'];
  }
  if (isset($element['delete']) and $element['delete']['#value']) {
    form_set_value($element['local_unsent_changes'], 2, $form_state);
  }
  // Validate address parts only if field is not empty.
  if (!care_fields_field_is_empty($element_values, array(
    'type' => 'care_email'))) {
    // Validate.
    if (!isset($element['delete']) or !$element['delete']['#value']) {
      if ($element['email']['#value'] == '') {
        form_error($element['email'], t('Email address is required for %name.', array(
          '%name' => $element['#title'])));
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
  $empty = ($not_in_care and ($delete or $empty_email));
  return $empty;
}

/**
 * Field datavabase schema.
 */
function _care_fields_care_email_schema() {
  $schema['columns']['communication_number'] = array(
    'type' => 'int',
    'size' => 'normal',
    'default' => 0,
    'not null' => TRUE);
  $schema['columns']['email'] = array(
    'type' => 'varchar',
    'length' => 255,
    'default' => '',
    'not null' => TRUE);
  $schema['columns']['device_code'] = array(
    'type' => 'varchar',
    'length' => 8,
    'default' => '',
    'not null' => TRUE);
  $schema['columns']['device_default'] = array(
    'type' => 'int',
    'size' => 'tiny',
    'default' => 0,
    'not null' => TRUE);
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
function care_fields_email_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['communication_number'] = array(
    'type' => 'text',
    'label' => t('Communication number'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['email'] = array(
    'type' => 'text',
    'label' => t('Email'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['device_code'] = array(
    'type' => 'text',
    'label' => t('Type'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['local_unsent_changes'] = array(
    'type' => 'integer',
    'label' => t('Local unsent changes'),
    'description' => t('0 = unchanged, 1 = pending changes, 2 = pending deletion'),
    'setter callback' => 'entity_property_verbatim_set');
}
