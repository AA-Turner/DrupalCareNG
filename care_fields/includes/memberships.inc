<?php

/**
 * @file
 * Functions to define a CARE memberships field.
 */

/**
 * Form definition.
 *
 * Display read-only memberships data in Edit forms.
 */
function _care_fields_memberships_form(&$element, $form_state, $field, $instance, $items, $delta) {
  if (count($items)) {
    $element['membership'] = [
      '#title' => t('Membership details'),
      '#type' => 'fieldset',
    ];
    $element['membership']['contact_number_ro'] = [
      '#title' => 'Membership number',
      '#type' => 'item',
      '#markup' => $items[$delta]['membership_number'],
    ];
    // Convert membership code into description.
    $membership_terms = taxonomy_get_term_by_name($items[$delta]['membership_type'], 'care_taxonomy_membership_code');
    $membership_term = array_shift($membership_terms);
    if ($membership_term) {
      $type = $membership_term->description;
    }
    else {
      $type = $items[$delta]['membership_type'];
    }
    $element['membership']['membership_type_ro'] = [
      '#title' => 'Membership type',
      '#type' => 'item',
      '#markup' => check_plain($type),
    ];
    $date = new DateTime($items[$delta]['renewal_date']);
    $element['membership']['renewal_date_ro'] = [
      '#title' => 'Renewal due',
      '#type' => 'item',
      '#markup' => format_date($date->getTimestamp(), 'custom', 'j M Y'),
    ];
    $element['membership_type'] = [
      '#type' => 'value',
      '#value' => $items[$delta]['membership_type'],
    ];
    $element['renewal_date'] = [
      '#type' => 'value',
      '#value' => $items[$delta]['renewal_date'],
    ];
    $element['membership_number'] = [
      '#type' => 'value',
      '#value' => $items[$delta]['membership_number'],
    ];
    $element['payment_plan_number'] = [
      '#type' => 'value',
      '#value' => $items[$delta]['payment_plan_number'],
    ];
    $element['payment_method'] = [
      '#type' => 'value',
      '#value' => $items[$delta]['payment_method'],
    ];
    $element['balance'] = [
      '#type' => 'value',
      '#value' => $items[$delta]['balance'],
    ];
    $element['refreshed'] = [
      '#type' => 'value',
      '#value' => $items[$delta]['refreshed'],
    ];
  }
  else {
    $element['no_memberships'] = [
      '#markup' => '<p>You do not have a current membership of Cycling UK, join today to enjoy the benefits of membership and to support our campaigning work</p>',
    ];
  }
}

/**
 * Field-is-empty test.
 */
function _care_fields_memberships_field_is_empty($item, $field) {
  return $item['membership_number'] == '';
}

/**
 * Field display formatter.
 */
function _care_memberships_formatter($item, $entity, $field) {
  $user_wrapper = entity_metadata_wrapper('user', $entity);
  if ($user_wrapper) {
    // Get user's contact number.
    $field_name = variable_get('care_user_membership_number_field', '');
    if ($field_name) {
      $contact_number = $user_wrapper->{$field_name}->number->value();
    }
  }
  else {
    $contact_number = 0;
  }
  $date = DateTime::createFromFormat('Y-m-d H:i:s', $item['renewal_date']);
  $type = $item['membership_type'];
  $membership_terms = taxonomy_get_term_by_name($type, 'care_taxonomy_membership_code');
  $membership_term = array_shift($membership_terms);
  if ($membership_term) {
    $type = $membership_term->description;
  }
  $renewal = format_date($date->getTimestamp(), 'custom', 'j M Y');
  $markup = t("%type, renewal due: %renewal", [
    '%type' => $type,
    '%renewal' => $renewal,
  ]);

  // Renewal link, if needed.
  if ($item['balance'] > 0) {
    if ($item['payment_method'] == 'DD') {
      $markup .= ' - ' . t('Payment by Direct Debit.');
    }
    elseif ($item['membership_type'] != 'Q') {
      if ($contact_number) {
        $markup .= '<div>' . l(t('Renew Now'), 'https://shop.cyclinguk.org/membership/renew/' . $contact_number . '/' . $item['membership_number'], array('attributes' => array('class' => array('btn btn-purple form-submit btn-margin')))) . '</div>';
      }
    }
  }

  // Allow other modules (e.g. care_user) to modify the membership display.
  drupal_alter('care_fields_membership_display', $markup, $item, $contact_number);

  return $markup;
}

/**
 * Field display formatter.
 */
function _care_memberships_simple_formatter($item) {
  $date = DateTime::createFromFormat('Y-m-d H:i:s', $item['renewal_date']);
  $type = $item['membership_type'];
  $membership_terms = taxonomy_get_term_by_name($type, 'care_taxonomy_membership_code');
  $membership_term = array_shift($membership_terms);
  if ($membership_term) {
    $type = $membership_term->description;
  }
  $renewal = format_date($date->getTimestamp(), 'custom', 'j M Y');
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'commerce_product');
  $query->entityCondition('bundle', 'membership');
  $query->fieldCondition('field_membership_code', 'value', $type, '=');
  $result = $query->execute();
  if ($result) {
    $product_data = reset($result['commerce_product']);
    $product = commerce_product_load($product_data->product_id);
    $type = $product->title;
  }
  $markup = t("%type, renewal due %renewal", [
    '%type' => $type,
    '%renewal' => $renewal,
  ]);
  return $markup;
}

/**
 * Database schema.
 */
function _care_fields_care_memberships_schema() {
  $schema['columns']['membership_type'] = [
    'type' => 'varchar',
    'length' => 50,
    'default' => '',
    'not null' => TRUE,
  ];
  $schema['columns']['renewal_date'] = [
    'type' => 'datetime',
    'mysql_type' => 'datetime',
    'pgsql_type' => 'timestamp without time zone',
    'sqlite_type' => 'varchar',
    'sqlsrv_type' => 'smalldatetime',
    'not null' => TRUE,
  ];
  $schema['columns']['membership_number'] = [
    'type' => 'int',
    'size' => 'normal',
    'default' => 0,
    'not null' => FALSE,
  ];
  $schema['columns']['payment_plan_number'] = [
    'type' => 'int',
    'size' => 'normal',
    'default' => 0,
    'not null' => FALSE,
  ];
  $schema['columns']['balance'] = [
    'type' => 'numeric',
    'precision' => 10,
    'scale' => 2,
    'default' => 0,
    'not null' => FALSE,
  ];
  $schema['columns']['payment_method'] = [
    'type' => 'varchar',
    'length' => 10,
    'default' => '',
    'not null' => FALSE,
  ];
  $schema['columns']['refreshed'] = [
    'type' => 'datetime',
    'mysql_type' => 'datetime',
    'pgsql_type' => 'timestamp without time zone',
    'sqlite_type' => 'varchar',
    'sqlsrv_type' => 'smalldatetime',
    'not null' => TRUE,
  ];
  return $schema;
}

/**
 * Entity integration.
 */
function care_fields_memberships_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['membership_type'] = [
    'type' => 'text',
    'label' => t('Membership type'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['renewal_date'] = [
    'type' => 'text',
    'label' => t('Renewal date'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['membership_number'] = [
    'type' => 'text',
    'label' => t('Membership number'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['payment_plan_number'] = [
    'type' => 'text',
    'label' => t('Payment Plan number'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['balance'] = [
    'type' => 'text',
    'label' => t('Balance'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['payment_method'] = [
    'type' => 'text',
    'label' => t('Payment Method'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
  $property['property info']['refreshed'] = [
    'type' => 'text',
    'label' => t('Refreshed date and time'),
    'setter callback' => 'entity_property_verbatim_set',
  ];
}
