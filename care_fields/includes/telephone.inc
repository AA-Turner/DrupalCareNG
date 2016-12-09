<?php

function _care_fields_telephone_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $element['communication_number'] = array(
    '#type' => 'value',
    '#value' => isset($items[$delta]['communication_number']) ? $items[$delta]['communication_number'] : 0);
  if ($field['cardinality'] == 1) {
    $element['prefix'] = array(
      '#type' => 'markup',
      '#markup' => '<fieldset class="form-wrapper"><legend><span class="fieldset-legend">' . $instance['label'] . '</legend><div class="fieldset-wrapper">');
  }
  $element['inline_start'] = array(
    '#markup' => '<div class="fieldset-wrapper container-inline">');
  $element['telephone'] = array(
    '#type' => 'textfield',
    '#title' => t('Telephone'),
    '#default_value' => isset($items[$delta]['telephone']) ? $items[$delta]['telephone'] : '',
    '#required' => $element['#required'],
    '#maxlength' => 20,
    '#size' => 30);
  $telephone_device_options = array();
  $telephone_devices = preg_split('/\r\n|[\r\n]/', variable_get('care_fields_telephone_devices', "TH|Home telephone\nTE|Work Telephone"));
  foreach ($telephone_devices as $index => $telephone_device) {
    if (trim($telephone_device)) {
      $data = explode("|", $telephone_device);
      $telephone_device_options[$data[0]] = $data[1];
    }
  }
  $telephone_device_default = isset($items[$delta]['device_code']) ? $items[$delta]['device_code'] : key($telephone_device_options);
  if (!in_array($telephone_device_default, array_keys($telephone_device_options))) {
    $telephone_device_options += array(
      $telephone_device_default => 'Device code ' . $telephone_device_default);
  }
  $element['device_code'] = array(
    '#type' => 'select',
    '#title' => t('Type'),
    '#options' => $telephone_device_options,
    '#default_value' => $telephone_device_default,
    '#required' => $element['#required']);
  if ($delta > 0) {
    $element['delete'] = array(
      '#type' => 'checkbox',
      '#title' => t('Delete this telephone number.'),
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
    '_care_fields_field_widget_telephone_validate');

}

function _care_fields_field_widget_telephone_validate(&$element, &$form_state) {
  $element_values = array(
    'communication_number' => $element['communication_number']['#value']);
  $field_names = array(
    'telephone',
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
    'type' => 'care_telephone'))) {
    // Validate.
    if (!isset($element['delete']) or !$element['delete']['#value']) {
      if ($element['telephone']['#value'] == '') {
        form_error($element['telephone'], t('Telephone number is required for %name.', array(
          '%name' => $element['#title'])));
      }
      elseif (strlen($element['telephone']['#value']) > 20) {
        form_error($element['telephone'], t('Telephone numbers cannot be longer than 20 characters.', array(
          '%name' => $element['#title'])));
      }
    }
  }
}

function _care_fields_telephone_field_is_empty($item, $field) {
  $not_in_care = ($item['communication_number'] == 0);
  $empty_telephone = ($item['telephone'] == '');
  $delete = (isset($item['delete']) and $item['delete']);
  $empty = ($not_in_care and ($delete or $empty_telephone));
  return $empty;
}

function care_fields_telephone_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['communication_number'] = array(
    'type' => 'text',
    'label' => t('Communication number'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['telephone'] = array(
    'type' => 'text',
    'label' => t('Telephone'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['device_code'] = array(
    'type' => 'text',
    'label' => t('Type'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['local_unsent_changes'] = array(
    'type' => 'boolean',
    'label' => t('Local unsent changes'),
    'setter callback' => 'entity_property_verbatim_set');
}

