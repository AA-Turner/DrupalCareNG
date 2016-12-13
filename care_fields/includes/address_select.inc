<?php
/**
 * @file
 * Definitions and callbacks for CARE address selector field.
 */

/**
 * Form definition.
 */
function _care_fields_address_select_form(&$element, $form_state, $field, $instance, $items, $delta) {
  // kpr($form_state);
  // kpr($field);
  // kpr($instance);
  // kpr($element);
  // dpm($items[$delta]);
  /*
   * Address numbers are used for addresses previously saved in CARE.
   * An address number of zero means an address for the user that has not yet
   *  been saved.
   * An address number of minus one means a new address entered here.
   */
  $number_field_name = "";
  foreach ($element['#field_parents'] as $index => $parent) {
    if ($index) {
      $number_field_name .= "[$parent]";
    }
    else {
      $number_field_name = $parent;
    }
  }
  if ($number_field_name) {
    $number_field_name .= "[" . $field['field_name'] . "]";
  }
  else {
    $number_field_name .= $field['field_name'];
  }
  $number_field_name .= "[" . $element['#language'] . "][" . $element['#delta'] . "][address_index]";
  // Get addresses from relevant user, if any.
  $waccount = NULL;
  switch ($instance['entity_type']) {
    case 'commerce_customer_profile':
      $account = NULL;
      if (isset($form_state['order'])) {
        $account = user_load($form_state['order']->uid);
      }
      elseif (isset($form_state['commerce_order'])) {
        $account = user_load($form_state['commerce_order']->uid);
      }
      elseif (isset($element['#entity']->uid)) {
        $account = user_load($element['#entity']->uid);
      }
      if ($account and $account->uid != 0) {
        $waccount = entity_metadata_wrapper('user', $account);
      }
      break;
  }
  // If we have a non-anonymous user, show their addresses as options.
  if ($waccount) {
    $address_options = array();
    $address_default = -1;
    $address_field_name = variable_get('care_user_address_field', '');
    $address_data = $waccount->$address_field_name->value();
    foreach ($address_data as $index => $address) {
      $compact_address = array();
      $compact_address[] = $address['address_line_1'];
      if ($address['address_line_2']) {
        $compact_address[] = $address['address_line_2'];
      }
      if ($address['address_line_3']) {
        $compact_address[] = $address['address_line_3'];
      }
      $compact_address[] = $address['town'];
      $compact_address[] = $address['postcode'];
      $address_options[$index] = implode(", ", $compact_address);
      // dpm($items, 'items');
      // dpm($address, 'address');
      $address_default = isset($items[$delta]['address_index']) ? $items[$delta]['address_index'] : 0;
      $element['address_data'] = array(
        '#type' => 'value',
        '#value' => $address_data);
    }
    $address_options[-1] = 'New Address';
    if (isset($items[$delta]) && $items[$delta]['address_number'] == -1) {
      $address_default = -1;
    }
    $element['address_index'] = array(
      '#type' => 'select',
      '#title' => 'Address',
      '#options' => $address_options,
      '#default_value' => $address_default);
  }
  else {
    $element['address_index'] = array(
      '#type' => 'value',
      '#value' => 0);
  }
  $element['address_number'] = array(
    '#type' => 'value',
    '#value' => -1);

  // Generate country field options.
  $countries = variable_get('care_fields_countries', 'UK|United Kingdom');
  $countries = preg_split('/\r\n|[\r\n]/', $countries);
  $country_default = isset($items[$delta]['country_code']) ? $items[$delta]['country_code'] : variable_get('care_fields_country_default', 'UK');
  $country_options = array();
  if ($country_default == '') {
    $country_options += array(
      '' => t(' - select - '));
  }
  foreach ($countries as $index => $country) {
    if (trim($country)) {
      $data = explode("|", $country);
      $country_options[$data[0]] = $data[1];
    }
  }

  $element['address_line_1'] = array(
    '#type' => 'textfield',
    '#title' => t('Address line 1'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $number_field_name . '"]' => array(
          'value' => -1))),
    '#default_value' => isset($items[$delta]['address_line_1']) ? $items[$delta]['address_line_1'] : '',
    '#size' => 30);
  $element['address_line_2'] = array(
    '#type' => 'textfield',
    '#title' => t('Address line 2'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $number_field_name . '"]' => array(
          'value' => -1))),
    '#default_value' => isset($items[$delta]['address_line_2']) ? $items[$delta]['address_line_2'] : '',
    '#size' => 30);
  $element['address_line_3'] = array(
    '#type' => 'textfield',
    '#title' => t('Address line 3'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $number_field_name . '"]' => array(
          'value' => -1))),
    '#default_value' => isset($items[$delta]['address_line_3']) ? $items[$delta]['address_line_3'] : '',
    '#size' => 30);
  $element['town'] = array(
    '#type' => 'textfield',
    '#title' => t('Town'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $number_field_name . '"]' => array(
          'value' => -1))),
    '#default_value' => isset($items[$delta]['town']) ? $items[$delta]['town'] : '',
    '#size' => 30);
  $element['county'] = array(
    '#type' => 'textfield',
    '#title' => t('County'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $number_field_name . '"]' => array(
          'value' => -1))),
    '#default_value' => isset($items[$delta]['county']) ? $items[$delta]['county'] : '',
    '#size' => 30);
  $element['country_code'] = array(
    '#type' => 'select',
    '#title' => t('Country'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $number_field_name . '"]' => array(
          'value' => -1))),
    '#options' => $country_options,
    '#default_value' => $country_default);
  $element['postcode'] = array(
    '#type' => 'textfield',
    '#title' => t('Postcode'),
    '#states' => array(
      'visible' => array(
        ':input[name="' . $number_field_name . '"]' => array(
          'value' => -1))),
    '#default_value' => isset($items[$delta]['postcode']) ? $items[$delta]['postcode'] : '',
    '#size' => 10,
    '#maxlength' => 10);
  $element['local_unsent_changes'] = array(
    '#type' => 'value',
    '#value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] : 0);
  $element['#element_validate'] = array(
    '_care_fields_field_widget_address_select_validate');

}

/**
 * Form validation.
 */
function _care_fields_field_widget_address_select_validate(&$element, &$form_state) {
  $address_index = $element['address_index']['#value'];
  if ($address_index === -1) {
    form_set_value($element['local_unsent_changes'], 1, $form_state);
    form_set_value($element['address_number'], 0, $form_state);
    if ($element['town']['#value'] == '') {
      form_error($element['town'], t('Town is required for %name', array(
        '%name' => $element['#title'])));
    }
  }
}
