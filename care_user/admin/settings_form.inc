<?php

function care_user_admin_settings_form($form, &$form_state) {

  $date_formats = array();
  $date_types = system_get_date_types();
  foreach ($date_types as $key => $value) {
    $date_formats[$value['type']] = t('@date_format format', array(
      '@date_format' => $value['title'])) . ': ' . format_date(REQUEST_TIME, $value['type']);
  }

  $form['care_user_message_date_format'] = array(
    '#title' => t('Date format to use'),
    '#description' => t('Select the format used to display membership renewal dates in user messages. Date formats are configured at !link.', array(
      '!link' => l(t('Configuration') . ' > ' . t('Regional and language') . ' > ' . t('Date and time'), 'admin/config/regional/date-time'))),
    '#type' => 'select',
    '#options' => $date_formats,
    '#default_value' => variable_get('care_user_message_date_format'));

  $form['care_user_keep_history'] = array(
    '#title' => t('Edited address/telephone/email record behaviour'),
    '#type' => 'radios',
    '#options' => array(
      1 => 'Keep edit history by hiding old CARE records and creating new CARE records.',
      0 => 'Edit existing CARE records in-place, to minimise new record creation.'),
    '#default_value' => variable_get('care_user_keep_history', TRUE));

  $form['care_user_rules_messages'] = array(
    '#title' => t('Email copying Rules action messages'),
    '#description' => t('Custom messages can be displayed, if required, using additional Rules actions.'),
    '#type' => 'radios',
    '#options' => array(
      1 => 'Display built-in messages when email-copying actions are executed.',
      0 => 'Do not display built-in messages for these actions.'),
    '#default_value' => variable_get('care_user_rules_messages', TRUE));

  $field_instances = field_info_instances('user', 'user');
  $care_membership_number_field_options = array(
    '' => t('Disable all synchronisation with CARE'));
  $care_memberships_field_options = array(
    '' => t('Do not synchronise with CARE'));
  $care_name_field_options = array(
    '' => t('Do not synchronise with CARE'));
  $care_date_field_options = array(
    '' => t('Do not synchronise with CARE'));
  $care_address_field_options = array(
    '' => t('Do not synchronise with CARE'));
  $care_telephone_field_options = array(
    '' => t('Do not synchronise with CARE'));
  $care_email_field_options = array(
    '' => t('Do not synchronise with CARE'));
  foreach ($field_instances as $field_name => $value) {
    $field_info = field_info_field($field_name);
    $option_display = $value['label'] . ' (' . $field_name . ')';
    if (in_array($field_info['type'], array(
      'care_address',
      'care_telephone',
      'care_email'))) {
      $option_display .= $field_info['cardinality'] == -1 ? ' - All CARE values' : ' - Default CARE value only';
    }
    switch ($field_info['type']) {
      case 'care_membership_number':
        $care_membership_number_field_options[$field_name] = $option_display;
        break;
      case 'care_memberships':
        $care_memberships_field_options[$field_name] = $option_display;
        break;
      case 'care_name':
        $care_name_field_options[$field_name] = $option_display;
        break;
      case 'care_date':
        $care_date_field_options[$field_name] = $option_display;
        break;
      case 'care_address':
        $care_address_field_options[$field_name] = $option_display;
        break;
      case 'care_telephone':
        $care_telephone_field_options[$field_name] = $option_display;
        break;
      case 'care_email':
        $care_email_field_options[$field_name] = $option_display;
        break;
    }
  }
  $form['field_mappings'] = array(
    '#title' => t('Field mappings'),
    '#type' => 'fieldset');
  $form['field_mappings']['care_user_membership_number_field'] = array(
    '#title' => t('Membership number (any care_membership_number field)'),
    '#type' => 'radios',
    '#options' => $care_membership_number_field_options,
    '#default_value' => variable_get('care_user_membership_number_field', ''));
  $form['field_mappings']['care_user_memberships_field'] = array(
    '#title' => t('Memberships (any care_memberships field)'),
    '#type' => 'radios',
    '#options' => $care_memberships_field_options,
    '#default_value' => variable_get('care_user_memberships_field', ''));
  $form['field_mappings']['care_user_name_field'] = array(
    '#title' => t('Name (any care_name field)'),
    '#type' => 'radios',
    '#options' => $care_name_field_options,
    '#default_value' => variable_get('care_user_name_field', ''));
  $form['field_mappings']['care_user_dob_field'] = array(
    '#title' => t('Date of birth (any care_date field)'),
    '#type' => 'radios',
    '#options' => $care_date_field_options,
    '#default_value' => variable_get('care_user_dob_field', ''));
  $form['field_mappings']['care_user_address_field'] = array(
    '#title' => t('Addresses (any care_address field)'),
    '#type' => 'radios',
    '#options' => $care_address_field_options,
    '#default_value' => variable_get('care_user_address_field', ''));
  $form['field_mappings']['care_user_telephone_field'] = array(
    '#title' => t('Telephone numbers (any care_telephone field)'),
    '#type' => 'radios',
    '#options' => $care_telephone_field_options,
    '#default_value' => variable_get('care_user_telephone_field', ''));
  $form['field_mappings']['care_user_email_field'] = array(
    '#title' => t('Email addresses (any care_email field)'),
    '#type' => 'radios',
    '#options' => $care_email_field_options,
    '#default_value' => variable_get('care_user_email_field', ''));

  return system_settings_form($form);
}