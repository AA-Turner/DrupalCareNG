<?php

function care_fields_admin_settings_form($form, &$form_state) {

  $form['care_fields_show_numbers'] = array(
    '#title' => t('Display CARE address/communication numbers to administrators.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('care_fields_show_numbers', TRUE));
  $form['titles'] = array(
    '#type' => 'fieldset',
    '#title' => 'Titles');
  $form['titles']['care_fields_titles'] = array(
    '#title' => t('Titles'),
    '#type' => 'textarea',
    '#default_value' => variable_get('care_fields_titles', "Mr\nMrs\nMiss\nMs\nDr"));
  $form['titles']['import'] = array(
    '#type' => 'submit',
    '#value' => 'Import titles from CARE',
    '#submit' => array(
      'care_fields_admin_titles_import'));
  $form['countries'] = array(
    '#type' => 'fieldset',
    '#title' => 'Countries');
  $form['countries']['care_fields_countries'] = array(
    '#title' => t('Countries'),
    '#type' => 'textarea',
    '#default_value' => variable_get('care_fields_countries', "UK|United Kingdom\nUSA|United States of America"));
  $form['countries']['care_fields_country_default'] = array(
    '#title' => t('Default Country code'),
    '#type' => 'textfield',
    '#size' => 8,
    '#default_value' => variable_get('care_fields_country_default', 'UK'));
  $form['countries']['import'] = array(
    '#type' => 'submit',
    '#value' => 'Import countries from CARE',
    '#submit' => array(
      'care_fields_admin_countries_import'));
  $form['devices'] = array(
    '#type' => 'fieldset',
    '#title' => 'Device types');
  $form['devices']['care_fields_email_devices'] = array(
    '#title' => t('Email'),
    '#type' => 'textarea',
    '#default_value' => variable_get('care_fields_email_devices', "EM|Home E-Mail\nEW|Work Email"));
  $form['devices']['care_fields_telephone_devices'] = array(
    '#title' => t('Telephone'),
    '#type' => 'textarea',
    '#default_value' => variable_get('care_fields_telephone_devices', "TH|Home telephone\nTE|Work Telephone"));
  $form['devices']['care_fields_website_devices'] = array(
    '#title' => t('Website'),
    '#type' => 'textarea',
    '#default_value' => variable_get('care_fields_website_devices', "W|Web Site Address"));
  $form['devices']['import'] = array(
    '#type' => 'submit',
    '#value' => 'Import devices from CARE',
    '#submit' => array(
      'care_fields_admin_devices_import'));
  $form = system_settings_form($form);
  $form['actions']['reset'] = array(
    '#type' => 'submit',
    '#value' => 'Reset to default',
    '#submit' => array(
      'care_fields_admin_reset'));
  return $form;
}

function care_fields_admin_reset() {
  variable_del('care_fields_titles');
  variable_del('care_fields_countries');
  variable_del('care_fields_country_default');
  variable_del('care_fields_email_devices');
  variable_del('care_fields_telephone_devices');
  variable_del('care_fields_website_devices');
}

function care_fields_admin_devices_import() {
  $email_devices = array();
  $telephone_devices = array();
  $website_devices = array();
  $data = array();
  $typedata = array(
    'pLookupDataType' => 'xldtDevices');
  $result = care_call_method('GetLookupData', $data, $typedata);
  foreach ($result as $row) {
    if ($row->Email == 'Y') {
      $email_devices[] = "$row->Device|$row->DeviceDesc";
    }
    elseif ($row->WwwAddress == 'Y') {
      $website_devices[] = "$row->Device|$row->DeviceDesc";
    }
    else {
      $telephone_devices[] = "$row->Device|$row->DeviceDesc";
    }
  }
  variable_set('care_fields_email_devices', implode("\n", $email_devices));
  variable_set('care_fields_telephone_devices', implode("\n", $telephone_devices));
  variable_set('care_fields_website_devices', implode("\n", $website_devices));
}

function care_fields_admin_titles_import() {
  $titles = array();
  $data = array();
  $typedata = array(
    'pLookupDataType' => 'xldtTitles');
  $result = care_call_method('GetLookupData', $data, $typedata);
  foreach ($result as $row) {
    $titles[] = trim($row->Title);
  }
  variable_set('care_fields_titles', implode("\n", $titles));
}

function care_fields_admin_countries_import() {
  $countries = array();
  $data = array();
  $typedata = array(
    'pLookupDataType' => 'xldtCountries');
  $result = care_call_method('GetLookupData', $data, $typedata);
  foreach ($result as $row) {
    $countries[] = "$row->Country|$row->CountryDesc";
  }
  variable_set('care_fields_countries', implode("\n", $countries));
}