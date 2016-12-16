<?php

/**
 * @file
 * Integration with Rules.
 */

/**
 * Implements hook_rules_action_info().
 */
function care_user_rules_action_info() {
  $actions = array(
    'care_user_fetch_all' => array(
      'label' => t('Fetch all user data from CARE.'),
      'group' => t('CARE'),
      'parameter' => array(
        'account' => array(
          'type' => 'user',
          'label' => t('User account to update with CARE data.')))),
    'care_user_fetch_memberships' => array(
      'label' => t('Fetch user’s membership data from CARE.'),
      'group' => t('CARE'),
      'parameter' => array(
        'account' => array(
          'type' => 'user',
          'label' => t('User account to update with CARE data.')))),
    'care_user_display_membership_status' => array(
      'label' => t('Display user’s membership status.'),
      'group' => t('CARE'),
      'parameter' => array(
        'account' => array(
          'type' => 'user',
          'label' => t('User account to display membership status for.')))),
    'care_user_save_all' => array(
      'label' => t('Save changed user data to CARE.'),
      'group' => t('CARE'),
      'parameter' => array(
        'account' => array(
          'type' => 'user',
          'label' => t('User account to save CARE data from.')))),
    'care_user_merge_drupal_mail_into_emails_field' => array(
      'label' => t('Merge/copy Drupal user account email into CARE emails field'),
      'group' => t('CARE'),
      'parameter' => array(
        'account' => array(
          'type' => 'user',
          'label' => t('User account to save CARE data from.')))),
    'care_user_care_email_field_to_drupal_mail' => array(
      'label' => t('Copy CARE default email to Drupal account mail.'),
      'group' => t('CARE'),
      'parameter' => array(
        'account' => array(
          'type' => 'user',
          'label' => t('User account to save CARE data from.')))));
  return $actions;
}

/**
 * Rules action to fetch all data from CARE into account.
 */
function care_user_fetch_all($account) {
  $care_user = new CareUser($account);
  $care_user->fetchFromCare();
}

/**
 * Rules action to fetch membership data from CARE into account.
 */
function care_user_fetch_memberships($account) {
  $care_user = new CareUser($account);
  $care_user->fetchMembershipDetails();
}

/**
 * Rules action to save all data to CARE from account.
 */
function care_user_save_all($account) {
  $care_user = new CareUser($account);
  $care_user->saveToCare();
}

/**
 * Rules action to display membership status for account.
 */
function care_user_display_membership_status($account) {
  $care_user = new CareUser($account);
  $memberships_field = $care_user->getMembershipsField();
  if ($care_user->getMembershipNumber() and $memberships_field) {
    // Refresh membership info from CARE.
    $care_user->fetchMembershipDetails();
    foreach ($memberships_field as $membership) {
      $renewaldate = DateTime::createFromFormat('Y-m-d', $membership->renewal_date->value());
      $token_values = array(
        '@type' => $membership->membership_type->value(),
        '@date' => format_date($renewaldate->getTimestamp(), variable_get('care_user_message_date_format', 'short')));
      if ($membership->balance->value() > 0) {
        if ($membership->renewal_date->value() < time()) {
          drupal_set_message(t('Your membership of type @type is overdue for renewal as of @date', $token_values));
        }
        else {
          drupal_set_message(t('Your membership of type @type is due for renewal before @date', $token_values));
        }
      }
      else {
        drupal_set_message(t('Your membership of type @type is current, renewal will be due before @date.', $token_values));
      }
      if ($membership->payment_method->value() == 'DD') {
        drupal_set_message(t('You are paying this membership by Direct Debit, so it will be renewed automatically.'));
      }
    }
  }
  else {
    drupal_set_message(t('You are not a member, or your website account is missing your membership number.'));
  }
}

/**
 * Rule action to copy Drupal account mail into CARE email field.
 */
function care_user_merge_drupal_mail_into_emails_field($account) {
  watchdog('care_user', 'care_user_merge_drupal_mail_into_emails_field for %user', array(
    '%user' => $account->name));
  $care_user = new CareUser($account);
  $drupal_email = $account->mail;
  $email_field = $care_user->getEmailField();
  $field_name = variable_get('care_user_email_field', '');
  $field_info = field_info_field($field_name);
  $field_instance = field_info_instance('user', $field_name, 'user');
  $multiple_value_field = ($field_info['cardinality'] == '-1');
  $existing = array();
  if ($multiple_value_field) {
    // Copy across if not already in the list.
    foreach ($email_field->getIterator() as $fred) {
      $existing[] = $fred->email->value();
    }
    $device_strings = preg_split('/\|/', variable_get('care_fields_email_devices', "EM|Home email\nEW|Work email"));
    $device_code_default = $device_strings[0];
    if (!in_array($drupal_email, $existing)) {
      $existing_emails = $email_field->value();
      // Add this as the first value, so it's the default.
      array_unshift($existing_emails, array(
        'communication_number' => 0,
        'email' => $drupal_email,
        'device_code' => $device_code_default,
        'device_default' => 1,
        'local_unsent_changes' => 1));
      $email_field->set($existing_emails);
      // Save field without saving the parent user.
      field_attach_update('user', $account);
      if (variable_get('care_user_rules_messages', TRUE)) {
        drupal_set_message(t('Website account email address %address added to %field field as new default.', array(
          '%address' => $drupal_email,
          '%field' => $field_instance['label'])));
      }
    }
    else {
      // Make existing email that matches the new default, if not already.
      $existing_emails = $email_field->value();
      if ($existing_emails[0]['email'] != $drupal_email) {
        foreach ($existing_emails as $index => $existing) {
          if ($existing['email'] == $drupal_email) {
            $default = $existing;
            $default['device_default'] = 1;
            unset($existing_emails[$index]);
            break;
          }
        }
        array_unshift($existing_emails, $default);
        $email_field->set($existing_emails);
        user_save($account);
        if (variable_get('care_user_rules_messages', TRUE)) {
          drupal_set_message(t('Website account email address %address set as new %field field default.', array(
            '%address' => $drupal_email,
            '%field' => $field_instance['label'])));
        }
      }
    }
  }
  else {
    // Copy across if not already the same.
    $current_care = $email_field->email->value();
    if ($current_care != $drupal_email) {
      $email_field->email->set($drupal_email);
      $email_field->local_unsent_changes->set(1);
      $email_field->device_default->set(1);
      // Save field without saving the parent user.
      field_attach_update('user', $account);
      if (variable_get('care_user_rules_messages', TRUE)) {
        drupal_set_message(t('Updated %field from %from to the website email address %to.', array(
          '%field' => $field_instance['label'],
          '%from' => $current_care,
          '%to' => $drupal_email)));
      }
    }
  }
}

/**
 * Rule action to copy CARE email into Drupal mail for account.
 */
function care_user_care_email_field_to_drupal_mail($account) {
  watchdog('care_user', 'care_user_care_email_field_to_drupal_mail for %user', array(
    '%user' => $account->name));
  $care_user = new CareUser($account);
  $email_field = $care_user->getEmailField();
  $drupal_email = $account->mail;
  $field_info = field_info_field(variable_get('care_user_email_field', ''));
  $multiple_value_field = ($field_info['cardinality'] == '-1');
  if ($multiple_value_field) {
    // Copy first entry to Drupal mail.
    $care_email = $email_field[0]->email->value();
    if ($care_email != $drupal_email) {
      $account->mail = $care_email;
      user_save($account);
      if (variable_get('care_user_rules_messages', TRUE)) {
        drupal_set_message(t('Website account email address updated from %from to %to', array(
          '%from' => $drupal_email,
          '%to' => $care_email)));
      }
    }
  }
  else {
    // Copy to Drupal mail.
    $care_email = $email_field->email->value();
    if ($care_email != $drupal_email) {
      $account->mail = $care_email;
      user_save($account);
      if (variable_get('care_user_rules_messages', TRUE)) {
        drupal_set_message(t('Website account email address updated from %from to %to', array(
          '%from' => $drupal_email,
          '%to' => $care_email)));
      }
    }
  }
}
