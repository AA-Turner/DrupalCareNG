<?php

/**
 * @file
 * Definitions and callbacks for CARE address selector field.
 */

/**
 * Form definition.
 */
function _care_fields_address_select_form(&$element, $form_state, $field, $instance, $items, $delta) {
  /*
   * Address numbers are used for addresses previously saved in CARE.
   * An address number of zero means an address for the user that has not yet
   * been saved.
   */
  $settings = $instance['settings']['input_options'];
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

  $element['prefix'] = [
    '#type' => 'markup',
    '#markup' => '<fieldset class="form-wrapper"><legend><span class="fieldset-legend">' . $instance['label'] . '</legend>',
  ];

  // If we have a non-anonymous user, show their addresses as options.
  if ($settings['default_from_current_user'] && user_is_logged_in()) {
    global $user;
    if ($user && $user->uid != 0) {
      $waccount = entity_metadata_wrapper('user', $user);
      $address_options = [];
      $address_default = -1;
      $address_field_name = variable_get('care_user_address_field', '');
      $address_data = $waccount->$address_field_name->value();
      $field_info = field_info_field($address_field_name);
      $multiple_value_field = ($field_info['cardinality'] == '-1');
      if (!$multiple_value_field && $address_data) {
        $multi_address_data = [
          0 => $address_data,
        ];
        $address_data = $multi_address_data;
      }
      $element['address_data'] = [
        '#type' => 'value',
        '#value' => $address_data,
      ];
      foreach ($address_data as $index => $address) {
        $compact_address = [];
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
        $address_default = isset($items[$delta]['address_index']) ? $items[$delta]['address_index'] : 0;
      }
      $address_options[(-1)] = 'New Address';
      $element['address_index'] = [
        '#type' => 'radios',
        '#title' => 'Address',
        '#options' => $address_options,
        '#default_value' => $address_default,
      ];
    }
    else {
      $element['address_index'] = [
        '#type' => 'value',
        '#value' => -1,
      ];
    }
  }
  else {
    $element['address_index'] = [
      '#type' => 'value',
      '#value' => -1,
    ];
  }
  $element['address_number'] = [
    '#type' => 'value',
    '#description' => 'CARE address number, or zero for a new address',
    '#value' => 0,
  ];
  // Generate country field options.
  $countries = variable_get('care_fields_countries', 'UK|United Kingdom');
  $countries = preg_split('/\r\n|[\r\n]/', $countries);
  $country_default = isset($items[$delta]['country_code']) ? $items[$delta]['country_code'] : variable_get('care_fields_country_default', 'UK');
  $country_options = [];
  if ($country_default == '') {
    $country_options += [
      '' => t(' - select - '),
    ];
  }
  foreach ($countries as $index => $country) {
    if (trim($country)) {
      $data = explode("|", $country);
      $country_options[$data[0]] = $data[1];
    }
  }

  $element['address_line_1'] = [
    '#type' => 'textfield',
    '#title' => t('Address line 1'),
    '#states' => [
      'visible' => [
        ':input[name="' . $number_field_name . '"]' => [
          'value' => -1,
        ],
      ],
      'required' => [
        ':input[name="' . $number_field_name . '"]' => [
          'value' => -1,
        ],
      ],
    ],
    '#default_value' => isset($items[$delta]['address_line_1']) ? $items[$delta]['address_line_1'] : '',
    '#size' => 30,
  ];
  $element['address_line_2'] = [
    '#type' => 'textfield',
    '#title' => t('Address line 2'),
    '#states' => [
      'visible' => [
        ':input[name="' . $number_field_name . '"]' => [
          'value' => -1,
        ],
      ],
    ],
    '#default_value' => isset($items[$delta]['address_line_2']) ? $items[$delta]['address_line_2'] : '',
    '#size' => 30,
  ];
  $element['address_line_3'] = [
    '#type' => 'textfield',
    '#title' => t('Address line 3'),
    '#states' => [
      'visible' => [
        ':input[name="' . $number_field_name . '"]' => [
          'value' => -1,
        ],
      ],
    ],
    '#default_value' => isset($items[$delta]['address_line_3']) ? $items[$delta]['address_line_3'] : '',
    '#size' => 30,
  ];
  $element['town'] = [
    '#type' => 'textfield',
    '#title' => t('Town'),
    '#states' => [
      'visible' => [
        ':input[name="' . $number_field_name . '"]' => [
          'value' => -1,
        ],
      ],
      'required' => [
        ':input[name="' . $number_field_name . '"]' => [
          'value' => -1,
        ],
      ],
    ],
    '#default_value' => isset($items[$delta]['town']) ? $items[$delta]['town'] : '',
    '#size' => 30,
  ];
  $element['county'] = [
    '#type' => 'textfield',
    '#title' => t('County'),
    '#states' => [
      'visible' => [
        ':input[name="' . $number_field_name . '"]' => [
          'value' => -1,
        ],
      ],
    ],
    '#default_value' => isset($items[$delta]['county']) ? $items[$delta]['county'] : '',
    '#size' => 30,
  ];

  $element['postcode'] = [
    '#type' => 'textfield',
    '#title' => t('Postcode'),
    '#states' => [
      'visible' => [
        ':input[name="' . $number_field_name . '"]' => [
          'value' => -1,
        ],
      ],
      'required' => [
        ':input[name="' . $number_field_name . '"]' => [
          'value' => -1,
        ],
      ],
    ],
    '#default_value' => isset($items[$delta]['postcode']) ? $items[$delta]['postcode'] : '',
    '#size' => 10,
    '#maxlength' => 10,
  ];
  $element['suffix'] = [
    '#type' => 'markup',
    '#markup' => '</div>',
  ];
  $element['local_unsent_changes'] = [
    '#type' => 'value',
    '#value' => isset($items[$delta]['local_unsent_changes']) ? $items[$delta]['local_unsent_changes'] : CARE_FIELDS_UNCHANGED,
  ];
  $element['#element_validate'] = [
    '_care_fields_field_widget_address_select_validate',
  ];

}

/**
 * Form validation.
 */
function _care_fields_field_widget_address_select_validate(&$element, &$form_state) {
  $address_index = $element['address_index']['#value'];
  if ($address_index == -1 && $element['#required']) {
    if (trim($element['address_line_1']['#value']) == '') {
      form_error($element['address_line_1'], t('%field is required for %name', [
        '%field' => t('Address line 1'),
        '%name' => $element['#title'],
      ]));
    }
    if (trim($element['town']['#value']) == '') {
      form_error($element['town'], t('%field is required for %name', [
        '%field' => t('Town'),
        '%name' => $element['#title'],
      ]));
    }
    if (trim($element['postcode']['#value']) == '') {
      form_error($element['postcode'], t('%field is required for %name', [
        '%field' => t('Postcode'),
        '%name' => $element['#title'],
      ]));
    }
  }
  if ($address_index == -1) {
    form_set_value($element['local_unsent_changes'], CARE_FIELDS_CHANGED, $form_state);
    form_set_value($element['address_number'], 0, $form_state);
  }
  else {
    $address_number = $element['address_data']['#value'][$address_index]['address_number'];
    form_set_value($element['address_number'], $address_number, $form_state);
  }
}
