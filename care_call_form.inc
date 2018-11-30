<?php

/**
 * @file
 * Administrator's utility form to call CARE web services.
 */

/**
 * Definition of form to test CARE calls from the administration pages.
 */
function care_call_form($form, &$form_state) {
  // Set defaults.
  if (!isset($form_state['values'])) {
    $form_state['values']['method'] = 'SelectContactData';
    $form_state['values']['parameters'] = '';
    $form_state['values']['getlookupdata_type'] = 'xldtTitles';
    $form_state['values']['selectcontactdata_type'] = 'xcdtContactInformation';
    $form_state['values']['other_method'] = 'SelectPaymentPlanData';
    $form_state['values']['other_type_name'] = 'pSelectDataType';
    $form_state['values']['other_type_value'] = 'xpdtPaymentPlanPayments';
  }
  $doc_root = variable_get('care_doc_root', 'http://127.0.0.1/careservices/documentation');

  $form['intro'] = [
    '#prefix' => '<p>',
    '#markup' => t('Use this form to test CARE calls. Refer to the !documentation for details of the required call types and parameters.', [
      '!documentation' => l(t('CARE API documentation'), $doc_root . '/WEBServicesSummary.htm'),
    ]),
    '#suffix' => '</p>',
  ];

  if (isset($form_state['care_call_result'])) {
    $form['care_call_result_display'] = [
      '#title' => t('CARE call result'),
      '#type' => 'textarea',
      '#rows' => 20,
      '#attributes' => [
        'class' => [
          'carexmlresult',
        ],
      ],
      '#value' => $form_state['care_call_result'],
    ];
  }

  $form['method'] = [
    '#title' => t('Call Method'),
    '#type' => 'radios',
    '#options' => [
      'SelectContactData' => t('SelectContactData (!doc)', [
        '!doc' => l(t('Documentation'), $doc_root . '/SelectContactData.htm'),
      ]),
      'GetLookupData' => t('GetLookupData (!doc)', [
        '!doc' => l(t('Documentation'), $doc_root . '/GetLookupData.htm'),
      ]),
      'FindContacts' => t('FindContacts (!doc)', [
        '!doc' => l(t('Documentation'), $doc_root . '/FindContacts.htm'),
      ]),
      'FindMembers' => t('FindMembers (!doc)', [
        '!doc' => l(t('Documentation'), $doc_root . '/FindMembers.htm'),
      ]),
      'other' => t('Other'),
    ],
    '#default_value' => $form_state['values']['method'],
  ];
  $form['selectcontactdata_type'] = [
    '#title' => t('SelectDataType'),
    '#type' => 'textfield',
    '#default_value' => $form_state['values']['selectcontactdata_type'],
    '#states' => [
      'visible' => [
        ':input[name="method"]' => [
          'value' => 'SelectContactData',
        ],
      ],
    ],
  ];
  $form['getlookupdata_type'] = [
    '#title' => t('LookupDataType'),
    '#type' => 'textfield',
    '#default_value' => $form_state['values']['getlookupdata_type'],
    '#states' => [
      'visible' => [
        ':input[name="method"]' => [
          'value' => 'GetLookupData',
        ],
      ],
    ],
  ];
  $form['other_method'] = [
    '#title' => t('Method Name'),
    '#type' => 'textfield',
    '#default_value' => $form_state['values']['other_method'],
    '#states' => [
      'visible' => [
        ':input[name="method"]' => [
          'value' => 'other',
        ],
      ],
    ],
  ];
  $form['other_type_name'] = [
    '#title' => t('Data Type Name'),
    '#type' => 'radios',
    '#options' => [
      'pSelectDataType' => 'SelectDataType',
      'pDataType' => 'DataType',
    ],
    '#description' => t('See documentation for the specific method, but this is usually SelectDataType.'),
    '#default_value' => $form_state['values']['other_type_name'],
    '#states' => [
      'visible' => [
        ':input[name="method"]' => [
          'value' => 'other',
        ],
      ],
    ],
  ];
  $form['other_type_value'] = [
    '#title' => t('Data Type Value'),
    '#type' => 'textfield',
    '#default_value' => $form_state['values']['other_type_value'],
    '#states' => [
      'visible' => [
        ':input[name="method"]' => [
          'value' => 'other',
        ],
      ],
    ],
  ];
  $form['parameters'] = [
    '#title' => t('XMLParameters'),
    '#description' => t('Enter parameters and values one per line, separated by a colon.'),
    '#type' => 'textarea',
    '#default_value' => $form_state['values']['parameters'],
  ];
  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Call CARE'),
  ];

  return $form;
}

/**
 * Form submit function.
 */
function care_call_form_submit($form, &$form_state) {
  $method = $form_state['values']['method'];
  $lines = preg_split("/\n/", $form_state['values']['parameters']);
  $data = [];
  $typedata = NULL;
  // Sort out parameter data.
  foreach ($lines as $line) {
    preg_match("/([^:]*):(.*)/", $line, $matches);
    if (isset($matches[1])) {
      $data[trim($matches[1])] = trim($matches[2]);
    }
  }
  // Sort out call type data.
  switch ($method) {
    case 'SelectContactData':
      $typedata = [
        'pSelectDataType' => $form_state['values']['selectcontactdata_type'],
      ];
      break;

    case 'GetLookupData':
      $typedata = [
        'pLookupDataType' => $form_state['values']['getlookupdata_type'],
      ];
      break;

    case 'other':
      $method = $form_state['values']['other_method'];
      if ($form_state['values']['other_type_name']) {
        $typedata = [
          $form_state['values']['other_type_name'] => $form_state['values']['other_type_value'],
        ];
      }
      break;
  }
  // Do the actual CARE method call.
  $result = care_call_method($method, $data, $typedata);
  $form_state['care_call_result'] = _care_pretty_xml($result);
  $form_state['rebuild'] = TRUE;
}
