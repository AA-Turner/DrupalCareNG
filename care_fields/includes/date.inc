<?php

/**
 * @file
 * Definitions and callbacks for CARE date field.
 */

/**
 * Form definition.
 */
function _care_fields_date_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $element['prefix'] = array(
    '#type' => 'markup',
    '#markup' => '<div class="container-inline">');
  if ($field['cardinality'] == 1) {
    $element['prefix'] = array(
      '#type' => 'markup',
      '#markup' => '<fieldset class="form-wrapper"><legend><span class="fieldset-legend">' . $instance['label'] . '</legend><div class="fieldset-wrapper container-inline">');
  }
  $current_year = date('Y');
  for ($y = -110; $y < 1; $y++) {
    $dob_years[$current_year + $y] = $current_year + $y;
  }
  $element['day'] = array(
    '#type' => 'select',
    '#options' => drupal_map_assoc(date_days($element['#required'])),
    '#default_value' => isset($items[$delta]['day']) ? $items[$delta]['day'] : '',
    '#required' => $element['#required']);
  $element['month'] = array(
    '#type' => 'select',
    '#options' => date_month_names($element['#required']),
    '#default_value' => isset($items[$delta]['month']) ? $items[$delta]['month'] : '',
    '#required' => $element['#required']);
  $element['year'] = array(
    '#type' => 'select',
    '#options' => $dob_years,
    '#default_value' => isset($items[$delta]['year']) ? $items[$delta]['year'] : '',
    '#required' => $element['#required']);
  if ($field['cardinality'] == 1) {
    $element['suffix'] = array(
      '#type' => 'markup',
      '#markup' => '</div>');
  }
  $element['local_unsent_changes'] = array(
    '#type' => 'value',
    '#value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] : 0);
  $element['#element_validate'] = array(
    '_care_fields_field_widget_date_validate');

}

/**
 * Form validation.
 */
function _care_fields_field_widget_date_validate(&$element, &$form_state) {
  $field_names = array(
    'day',
    'month',
    'year');
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
  // Validate date parts only if field is not empty.
  if (!care_fields_field_is_empty($element_values, array(
    'type' => 'care_date'))) {
    // Validate.
    if (!checkdate($element['month']['#value'], $element['day']['#value'], $element['year']['#value'])) {
      form_error($element['day'], t('Invalid date for %name.', array(
        '%name' => $element['#title'])));
    }
  }
}

/**
 * Field is empty test.
 */
function _care_fields_date_field_is_empty($item, $field) {
  $empty = ($item['day'] == '');
  return $empty;
}

/**
 * Database schema.
 */
function _care_fields_care_date_schema() {
  $schema['columns']['day'] = array(
    'type' => 'int',
    'size' => 'tiny',
    'default' => 0,
    'not null' => TRUE);
  $schema['columns']['month'] = array(
    'type' => 'int',
    'size' => 'tiny',
    'default' => 0,
    'not null' => TRUE);
  $schema['columns']['year'] = array(
    'type' => 'int',
    'size' => 'small',
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
function care_fields_date_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['day'] = array(
    'type' => 'text',
    'label' => t('Day'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['month'] = array(
    'type' => 'text',
    'label' => t('Month'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['year'] = array(
    'type' => 'text',
    'label' => t('Year'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['local_unsent_changes'] = array(
    'type' => 'integer',
    'label' => t('Local unsent changes'),
    'description' => t('0 = unchanged, 1 = pending changes, 2 = pending deletion'),
    'setter callback' => 'entity_property_verbatim_set');
}
