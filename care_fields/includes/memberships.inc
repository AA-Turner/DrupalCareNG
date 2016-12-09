<?php

/**
 * @file
 * Functions to define a CARE memberships field.
 */

function _care_fields_memberships_form(&$element, $form_state, $field, $instance, $items, $delta) {
  $element['placeholder'] = array(
    '#type' => 'markup',
    '#title' => $element['#title'],
    '#markup' => '<p>Memberships read-only</p>');
}

function _care_fields_memberships_field_is_empty($item, $field) {
  return TRUE;
}

function _care_memberships_formatter($item) {
  $date = DateTime::createFromFormat('Y-m-d H:i:s', $item['renewal_date']);
  $type = $item['membership_type'];
  $renewal = format_date($date->getTimestamp(), 'custom', 'j M Y');
  $refreshed = DateTime::createFromFormat('Y-m-d H:i:s', $item['refreshed']);
  $last_refreshed = format_date($refreshed->getTimestamp(), 'custom', 'j M Y, H:i:s');
  $markup = t("Membership type: %type, renewal due %renewal, payment type %payment, refreshed %refreshed", array(
    '%type' => $type,
    '%renewal' => $renewal,
    '%payment' => $item['payment_method'],
    '%refreshed' => $last_refreshed));
  return $markup;
}

function care_fields_memberships_property_info_callback(&$info, $entity_type, $field, $instance, $field_type) {
  $property = &$info[$entity_type]['bundles'][$instance['bundle']]['properties'][$field['field_name']];
  $property['getter callback'] = 'entity_metadata_field_verbatim_get';
  $property['setter callback'] = 'entity_metadata_field_verbatim_set';
  unset($property['query callback']);
  $property['property info']['membership_type'] = array(
    'type' => 'text',
    'label' => t('Membership type'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['renewal_date'] = array(
    'type' => 'text',
    'label' => t('Renewal date'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['membership_number'] = array(
    'type' => 'text',
    'label' => t('Membership number'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['payment_plan_number'] = array(
    'type' => 'text',
    'label' => t('Payment Plan number'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['balance'] = array(
    'type' => 'text',
    'label' => t('Balance'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['payment_method'] = array(
    'type' => 'text',
    'label' => t('Payment Method'),
    'setter callback' => 'entity_property_verbatim_set');
  $property['property info']['refreshed'] = array(
    'type' => 'text',
    'label' => t('Refreshed date and time'),
    'setter callback' => 'entity_property_verbatim_set');
}
