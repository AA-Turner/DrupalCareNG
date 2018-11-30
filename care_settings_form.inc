<?php

/**
 * @file
 * CARE module settings form.
 */

/**
 * Drupal Form API function to generate settings form.
 */
function care_settings_form($form, &$form_state) {

  $form['care_database_to_use'] = [
    '#title' => t('Database to use'),
    '#type' => 'radios',
    '#options' => [
      'live' => 'Live Database',
      'test' => 'Test Database',
    ],
    '#default_value' => variable_get('care_database_to_use', 'test'),
  ];

  $form['live'] = [
    '#title' => t('Live Database Connection'),
    '#type' => 'fieldset',
    '#tree' => FALSE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];

  $form['live']['care_wsdl_url'] = [
    '#title' => t('WSDL URL'),
    '#type' => 'textfield',
    '#description' => t('Use the button below to check the URL without saving it.'),
    '#length' => 50,
    '#default_value' => variable_get('care_wsdl_url', 'http://127.0.0.1/careservices/dataaccess.asmx?WSDL'),
  ];

  $form['live']['ntlm'] = [
    '#title' => t('NTLM authentication'),
    '#type' => 'fieldset',
    '#description' => t('Leave these blank unless connection requires NTLM authentication.'),
    '#tree' => FALSE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];
  $form['live']['ntlm']['care_ntlm_username'] = [
    '#title' => t('NTLM username'),
    '#type' => 'textfield',
    '#length' => 50,
    '#default_value' => variable_get('care_ntlm_username', ''),
  ];
  $form['live']['ntlm']['care_ntlm_password'] = [
    '#title' => t('NTLM password'),
    '#type' => 'textfield',
    '#length' => 50,
    '#default_value' => variable_get('care_ntlm_password', ''),
  ];
  $form['live']['check_live_wsdl'] = [
    '#value' => t('Check live WDSL URL'),
    '#type' => 'submit',
    '#submit' => [
      'care_settings_form_check_live_wsdl',
    ],
  ];

  $form['test'] = [
    '#title' => t('Test Database Connection'),
    '#type' => 'fieldset',
    '#tree' => FALSE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];

  $form['test']['care_test_wsdl_url'] = [
    '#title' => t('WSDL URL'),
    '#type' => 'textfield',
    '#description' => t('Use the button below to check the URL without saving it.'),
    '#length' => 50,
    '#default_value' => variable_get('care_test_wsdl_url', 'http://127.0.0.1/careservices/dataaccess.asmx?WSDL'),
  ];

  $form['test']['ntlm'] = [
    '#title' => t('NTLM authentication'),
    '#type' => 'fieldset',
    '#description' => t('Leave these blank unless connection requires NTLM authentication.'),
    '#tree' => FALSE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];
  $form['test']['ntlm']['care_test_ntlm_username'] = [
    '#title' => t('NTLM username'),
    '#type' => 'textfield',
    '#length' => 50,
    '#default_value' => variable_get('care_test_ntlm_username', ''),
  ];
  $form['test']['ntlm']['care_test_ntlm_password'] = [
    '#title' => t('NTLM password'),
    '#type' => 'textfield',
    '#length' => 50,
    '#default_value' => variable_get('care_test_ntlm_password', ''),
  ];
  $form['test']['check_test_wsdl'] = [
    '#value' => t('Check test WDSL URL'),
    '#type' => 'submit',
    '#submit' => [
      'care_settings_form_check_test_wsdl',
    ],
  ];

  $form['care_doc_root'] = [
    '#title' => t('CARE documentation URL'),
    '#type' => 'textfield',
    '#description' => t('Home page for CARE API documentation.'),
    '#length' => 50,
    '#default_value' => variable_get('care_doc_root', 'http://127.0.0.1/careservices/documentation'),
  ];

  $form['logging'] = [
    '#title' => 'Logging Options',
    '#type' => 'fieldset',
  ];

  $form['logging']['care_log_calls'] = [
    '#title' => 'Log calls to CARE',
    '#type' => 'radios',
    '#options' => [
      'full' => 'Full logging',
      'part' => 'Partial logging, personal data obfuscated/redacted',
      'none' => 'No logging',
    ],
    '#default_value' => variable_get('care_log_calls', 'none'),
  ];

  $form['logging']['care_log_results'] = [
    '#title' => 'Log results from CARE',
    '#type' => 'radios',
    '#options' => [
      'full' => 'Full logging',
      'part' => 'Partial logging, personal data obfuscated/redacted',
      'none' => 'No logging',
    ],
    '#default_value' => variable_get('care_log_results', 'none'),
  ];

  return system_settings_form($form);
}

/**
 * Drupal Form API validation for the settings form.
 */
function care_settings_form_check_live_wsdl($form, &$form_state) {
  $url = $form_state['values']['care_wsdl_url'];
  $service_username = $form_state['input']['care_ntlm_username'];
  $service_password = $form_state['input']['care_ntlm_password'];
  $use_ntlm = ($service_username && $service_password);
  try {
    if (class_exists('SoapClient')) {
      if ($use_ntlm) {
        $client = new NtlmSoapClient($url, [
          'user' => $service_username,
          'password' => $service_password,
          'testwsdl' => TRUE,
        ]);
      }
      else {
        $client = new SoapClient($url);
      }
    }
    else {
      throw new RuntimeException('SoapClient class does not exist: please enable the PHP SOAP extension.');
    }
    if ($url == variable_get('care_wsdl_url', '') && $service_username == variable_get('care_ntlm_username') && $service_password == variable_get('care_ntlm_password')) {
      drupal_set_message(t('CARE live WSDL URL %url is OK, and is the current saved value.', [
        '%url' => $url,
      ]));

    }
    else {
      $form_state['rebuild'] = TRUE;
      drupal_set_message(t('CARE live WSDL URL %url is OK, but is not yet saved.', [
        '%url' => $url,
      ]));
      drupal_set_message(t('Click %savebutton to use this new URL.', [
        '%savebutton' => t('Save configuration'),
      ]), 'warning');
    }
  }

  catch
  (Exception $e) {
    $message = t('CARE live WSDL URL %url failed.', [
      '%url' => $url,
    ]);
    if ($use_ntlm) {
      $message .= t(' Using NTLM username %user and password %pass.', [
        '%user' => $service_username,
        '%pass' => $service_password,
      ]);
    }
    drupal_set_message($message, 'error');
    drupal_set_message($e->getMessage(), 'error');
    drupal_set_message(t('Reverted URL to current saved value.'), 'error');
  }
}

/**
 * Drupal Form API validation for the settings form.
 */
function care_settings_form_check_test_wsdl($form, &$form_state) {
  $url = $form_state['values']['care_test_wsdl_url'];
  $service_username = $form_state['input']['care_test_ntlm_username'];
  $service_password = $form_state['input']['care_test_ntlm_password'];
  $use_ntlm = ($service_username && $service_password);
  try {
    if (class_exists('SoapClient')) {
      if ($use_ntlm) {
        $client = new NtlmSoapClient($url, [
          'user' => $service_username,
          'password' => $service_password,
          'testwsdl' => TRUE,
        ]);
      }
      else {
        $client = new SoapClient($url);
      }
    }
    else {
      throw new RuntimeException('SoapClient class does not exist: please enable the PHP SOAP extension.');
    }
    if ($url == variable_get('care_test_wsdl_url', '')) {
      drupal_set_message(t('CARE test WSDL URL %url is OK, and is the current saved value.', [
        '%url' => $url,
      ]));
    }
    else {
      $form_state['rebuild'] = TRUE;
      drupal_set_message(t('CARE test WSDL URL %url is OK, but is not yet saved.', [
        '%url' => $url,
      ]));
      drupal_set_message(t('Click %savebutton to use this new URL.', [
        '%savebutton' => t('Save configuration'),
      ]), 'warning');
    }
  }
  catch (Exception $e) {
    drupal_set_message(t('CARE test WSDL URL %url failed.', [
      '%url' => $url,
    ]), 'error');
    drupal_set_message($e->getMessage(), 'error');
    drupal_set_message(t('Reverted test URL to current saved value.'), 'error');
  }
}
