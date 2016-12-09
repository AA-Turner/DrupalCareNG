<?php

function _care_fields_name_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $titles = variable_get('care_fields_titles', "Mr\nMrs\nMiss\nMs\nDr");
  $titles = preg_split('/\r\n|[\r\n]/', $titles);
  $title_default = isset($items[$delta]['title']) ? $items[$delta]['title'] : '';
  $title_options = array();
  if ($title_default == '') {
    $title_options += array(
      '' => t(' - select - '));
  }
  elseif (!in_array($title_default, $titles)) {
    $title_options += array(
      $title_default => $title_default);
  }
  $title_options += drupal_map_assoc($titles);
  $element['prefix'] = array(
    '#type' => 'markup',
    '#markup' => '<div class="container-inline">');
  if ($field['cardinality'] == 1) {
    $element['prefix'] = array(
      '#type' => 'markup',
      '#markup' => '<fieldset class="form-wrapper"><legend><span class="fieldset-legend">' . $instance['label'] . '</legend><div class="fieldset-wrapper">');
  }
  $element['title'] = array(
    '#type' => 'select',
    '#title' => t('Title'),
    '#default_value' => isset($items[$delta]['title']) ? $items[$delta]['title'] : '',
    '#required' => $element['#required'],
    '#options' => $title_options);
  $element['forenames'] = array(
    '#type' => 'textfield',
    '#title' => t('Forename(s)'),
    '#default_value' => isset($items[$delta]['forenames']) ? $items[$delta]['forenames'] : '',
    '#required' => $element['#required'],
    '#size' => 30,
    '#maxlength' => 60);
  $element['surname'] = array(
    '#type' => 'textfield',
    '#title' => t('Surname'),
    '#default_value' => isset($items[$delta]['surname']) ? $items[$delta]['surname'] : '',
    '#required' => $element['#required'],
    '#size' => 30,
    '#maxlength' => 50);
  $element['suffix'] = array(
    '#type' => 'markup',
    '#markup' => '</div>');
  $element['local_unsent_changes'] = array(
    '#type' => 'value',
    '#value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] : 0);
  $element['#element_validate'] = array(
    '_care_fields_field_widget_name_validate');
}

function _care_fields_field_widget_name_validate(&$element, &$form_state) {
  $field_names = array(
    'title',
    'forenames',
    'surname');
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
}

function _care_fields_name_field_is_empty($item, $field) {
  $empty = ($item['title'] == '' and $item['forenames'] == '' and $item['surname'] == '');
  return $empty;
}

function care_fields_name_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['title'] = array(
    'type' => 'text',
    'label' => t('Title'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['forenames'] = array(
    'type' => 'text',
    'label' => t('Fornames'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['surname'] = array(
    'type' => 'text',
    'label' => t('Surname'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['local_unsent_changes'] = array(
    'type' => 'boolean',
    'label' => t('Local unsent changes'),
    'setter callback' => 'entity_property_verbatim_set');
}

