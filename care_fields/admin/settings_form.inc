<?php

/**
 * @file
 * Administration settings form.
 */

/**
 * Form definition.
 */
function care_fields_admin_settings_form($form, &$form_state) {

  $form['care_fields_show_numbers'] = [
    '#title' => t('Display CARE address/communication numbers to administrators.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('care_fields_show_numbers', TRUE),
  ];
  $form['titles'] = [
    '#type' => 'fieldset',
    '#title' => 'Titles',
  ];
  $form['titles']['care_fields_titles'] = [
    '#title' => t('Titles'),
    '#type' => 'textarea',
    '#default_value' => variable_get('care_fields_titles', "Mr\nMrs\nMiss\nMs\nDr"),
  ];
  $form['titles']['import'] = [
    '#type' => 'submit',
    '#value' => 'Import titles from CARE',
    '#submit' => [
      'care_fields_admin_titles_import',
    ],
  ];
  $form['countries'] = [
    '#type' => 'fieldset',
    '#title' => 'Countries',
  ];
  $form['countries']['care_fields_countries'] = [
    '#title' => t('Countries'),
    '#type' => 'textarea',
    '#default_value' => variable_get('care_fields_countries', "UK|United Kingdom\nUSA|United States of America"),
  ];
  $form['countries']['care_fields_country_default'] = [
    '#title' => t('Default Country code'),
    '#type' => 'textfield',
    '#size' => 8,
    '#default_value' => variable_get('care_fields_country_default', 'UK'),
  ];
  $form['countries']['import'] = [
    '#type' => 'submit',
    '#value' => 'Import countries from CARE',
    '#submit' => [
      'care_fields_admin_countries_import',
    ],
  ];
  $form['devices'] = [
    '#type' => 'fieldset',
    '#title' => 'Device types',
  ];
  $form['devices']['care_fields_email_devices'] = [
    '#title' => t('Email'),
    '#type' => 'textarea',
    '#default_value' => variable_get('care_fields_email_devices', "EM|Home E-Mail\nEW|Work Email"),
  ];
  $form['devices']['care_fields_telephone_devices'] = [
    '#title' => t('Telephone'),
    '#type' => 'textarea',
    '#default_value' => variable_get('care_fields_telephone_devices', "TH|Home telephone\nTE|Work Telephone"),
  ];
  $form['devices']['care_fields_website_devices'] = [
    '#title' => t('Website'),
    '#type' => 'textarea',
    '#default_value' => variable_get('care_fields_website_devices', "W|Web Site Address"),
  ];
  $form['devices']['import'] = [
    '#type' => 'submit',
    '#value' => 'Import devices from CARE',
    '#submit' => [
      'care_fields_admin_devices_import',
    ],
  ];
  $form = system_settings_form($form);
  $form['actions']['reset'] = [
    '#type' => 'submit',
    '#value' => 'Reset to default',
    '#submit' => [
      'care_fields_admin_reset',
    ],
  ];
  return $form;
}

/**
 * Reset-all-values callback.
 */
function care_fields_admin_reset() {
  variable_del('care_fields_titles');
  variable_del('care_fields_countries');
  variable_del('care_fields_country_default');
  variable_del('care_fields_email_devices');
  variable_del('care_fields_telephone_devices');
  variable_del('care_fields_website_devices');
}

/**
 * Import-communications-devices callback.
 */
function care_fields_admin_devices_import() {
  $email_devices = [];
  $telephone_devices = [];
  $website_devices = [];
  $data = [
    'Active' => 'Y',
    'LookupGroup' => 'WEBD',
  ];
  $typedata = [
    'pLookupDataType' => 'xldtDevices',
  ];
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
  drupal_set_message(t('Imported !n (!e email, !t telephone, !w web) device types from CARE.', [
    '!n' => count($email_devices) + count($telephone_devices) + count($website_devices),
    '!e' => count($email_devices),
    '!t' => count($telephone_devices),
    '!w' => count($website_devices),
  ]));

  variable_set('care_fields_email_devices', implode("\n", $email_devices));
  variable_set('care_fields_telephone_devices', implode("\n", $telephone_devices));
  variable_set('care_fields_website_devices', implode("\n", $website_devices));
}

/**
 * Import-title-options callback.
 */
function care_fields_admin_titles_import() {
  $titles = [];
  $data = [
    'Active' => 'Y',
    'LookupGroup' => 'WEBT',
  ];
  $typedata = [
    'pLookupDataType' => 'xldtTitles',
  ];
  $result = care_call_method('GetLookupData', $data, $typedata);
  foreach ($result as $row) {
    $titles[] = trim($row->Title);
  }
  drupal_set_message(t('Imported !n titles from CARE.', [
    '!n' => count($titles),
  ]));
  variable_set('care_fields_titles', implode("\n", $titles));
}

/**
 * Import-country-options callback.
 */
function care_fields_admin_countries_import() {
  $countries = [];
  $data = [];
  $typedata = [
    'pLookupDataType' => 'xldtCountries',
  ];
  $result = care_call_method('GetLookupData', $data, $typedata);
  foreach ($result as $row) {
    $countries[] = "$row->Country|$row->CountryDesc";
  }
  variable_set('care_fields_countries', implode("\n", $countries));
}
