<?php

/**
 * @file
 * Define the CareUser class.
 */

/**
 * Class CareUser connects Drupal account to CARE.
 */
class CareUser {

  protected $account;

  protected $membershipNumber = 0;

  protected $membershipNumberField;

  protected $nameField;

  protected $dobField;

  protected $addressField;

  protected $emailField;

  protected $telephoneField;

  protected $membershipsField;

  /**
   * Construct a CareUser object from a Drupal account.
   */
  public function __construct($drupal_account) {
    $this->account = entity_metadata_wrapper('user', $drupal_account);
    $membership_number_field_name = variable_get('care_user_membership_number_field', '');
    if ($membership_number_field_name && isset($this->account->{$membership_number_field_name})) {
      $this->membershipNumberField = $this->account->{$membership_number_field_name};
      if ($this->account->{$membership_number_field_name}->value()) {
        $this->membershipNumber = $this->membershipNumberField->number->value();
      }
    }
    $name_field_name = variable_get('care_user_name_field', '');
    if ($name_field_name && isset($this->account->{$name_field_name})) {
      $this->nameField = $this->account->{$name_field_name};
    }
    $dob_field_name = variable_get('care_user_dob_field', '');
    if ($dob_field_name && isset($this->account->{$dob_field_name})) {
      $this->dobField = $this->account->{$dob_field_name};
    }
    $address_field_name = variable_get('care_user_address_field', '');
    if ($address_field_name && isset($this->account->$address_field_name)) {
      $this->addressField = $this->account->{$address_field_name};
    }
    $email_field_name = variable_get('care_user_email_field', '');
    if ($email_field_name && isset($this->account->$email_field_name)) {
      $this->emailField = $this->account->{$email_field_name};
    }
    $telephone_field_name = variable_get('care_user_telephone_field', '');
    if ($telephone_field_name && isset($this->account->$telephone_field_name)) {
      $this->telephoneField = $this->account->{$telephone_field_name};
    }
    $memberships_field_name = variable_get('care_user_memberships_field', '');
    if ($memberships_field_name && isset($this->account->$memberships_field_name)) {
      $this->membershipsField = $this->account->{$memberships_field_name};
    }
  }

  /**
   * Return the account's CARE membership number, if any.
   */
  public function getMembershipNumber() {
    return $this->membershipNumber;
  }

  /**
   * Return the name field as a wrapped entity.
   */
  public function getNameField() {
    return $this->nameField;
  }

  /**
   * Return the DoB field as a wrapped entity.
   */
  public function getDobField() {
    return $this->dobField;
  }

  /**
   * Return the email field as a wrapped entity.
   */
  public function getEmailField() {
    return $this->emailField;
  }

  /**
   * Return the telephone field as a wrapped entity.
   */
  public function getTelephoneField() {
    return $this->telephoneField;
  }

  /**
   * Return the address field as a wrapped entity.
   */
  public function getAddressField() {
    return $this->addressField;
  }

  /**
   * Return the memberships field as a wrapped entity.
   */
  public function getMembershipsField() {
    return $this->membershipsField;
  }

  /**
   * Add an email address, if it is new, avoiding duplicates.
   */
  public function addEmailFromField($care_email_field) {
    if ($care_email_field && $care_email_field->value()) {
      $field_info = field_info_field(variable_get('care_user_email_field', ''));
      $multiple_value_field = ($field_info['cardinality'] != 1);
      $supplied_email_address = $care_email_field->email->value();
      if ($multiple_value_field) {
        $emails = $this->emailField->value();
        // Do nothing if this email already exists.
        foreach ($emails as $email) {
          $existing_email_address = $email['email'];
          if ($existing_email_address == $supplied_email_address) {
            return;
          }
        }
        // Add new email address.
        $this->emailField[] = $care_email_field->value();
      }
      else {
        $existing_email_address = $this->emailField->email->value();
        // Do nothing if this email already exists.
        if ($existing_email_address == $supplied_email_address) {
          return;
        }
        // Change email address to new one.
        $this->emailField = $care_email_field->value();
        $this->emailField->communication_number = 0;
        $this->emailField->local_unsent_changes = CARE_FIELDS_CHANGED;
      }
      $this->saveFieldsToDrupal();
    }
  }

  /**
   * Add a possibly-new string-only email address.
   */
  public function addEmail($supplied_email_address) {
    $email_devices = preg_split('/\r\n|[\r\n]/', variable_get('care_fields_email_devices', "EM|Home email\nEW|Work email"));
    $email_device_options = [];
    foreach ($email_devices as $index => $email_device) {
      if (trim($email_device)) {
        $data = explode("|", $email_device);
        $email_device_options[$data[0]] = $data[1];
      }
    }
    $device_code_default = key($email_device_options);
    if ($supplied_email_address) {
      $field_info = field_info_field(variable_get('care_user_email_field', ''));
      $multiple_value_field = ($field_info['cardinality'] != 1);
      if ($multiple_value_field) {
        $emails = $this->emailField->value();
        // Do nothing if this email already exists.
        foreach ($emails as $email) {
          $existing_email_address = $email['email'];
          if (strtolower($existing_email_address) == strtolower($supplied_email_address)) {
            return;
          }
        }
        // Add new email address.
        $this->emailField[] = [
          'communication_number' => 0,
          'email' => $supplied_email_address,
          'device_code' => $device_code_default,
          'device_default' => 0,
          'local_unsent_changes' => CARE_FIELDS_CHANGED,
        ];
      }
      else {
        $existing_email_address = $this->emailField->email->value();
        // Do nothing if this email already exists.
        if (strtolower($existing_email_address) == strtolower($supplied_email_address)) {
          return;
        }
        // Change email address to new one.
        $this->emailField = [
          'communication_number' => 0,
          'email' => $supplied_email_address,
          'device_code' => $device_code_default,
          'device_default' => 0,
          'local_unsent_changes' => CARE_FIELDS_CHANGED,
        ];
      }
      $this->saveFieldsToDrupal();
    }

  }

  /**
   * Add an address, if it is new, avoiding duplicates.
   */
  public function addAddressFromField($care_address_field) {
    $field_info = field_info_field(variable_get('care_user_address_field', ''));
    $multiple_value_field = ($field_info['cardinality'] != 1);
    $supplied_address = $care_address_field->value();
    if ($multiple_value_field) {
      // Do nothing if this address already exists.
      foreach ($this->addressField as $address) {
        $existing_address = $address->value();
        if ($this->compareAddresses($existing_address, $supplied_address)) {
          return;
        }
      }
      $this->addressField[] = $supplied_address;
    }
    else {
      $existing_address = $this->addressField->value();
      // Do nothing if this address already exists.
      if ($this->compareAddresses($existing_address, $supplied_address)) {
        return;
      }
      // Change address to new one.
      $this->addressField = $supplied_address;
      $this->addressField->address_number = 0;
      $this->addressField->local_unsent_changes = CARE_FIELDS_CHANGED;
    }
    // Save fields without saving the parent user.
    $this->saveFieldsToDrupal();
  }

  /**
   * Get an AddressNumber for an address, if known.
   */
  public function getAddressNumberFromField($care_address_field) {
    $field_info = field_info_field(variable_get('care_user_address_field', ''));
    $multiple_value_field = ($field_info['cardinality'] == '-1');
    $given_address = $care_address_field->value();
    if ($multiple_value_field) {
      foreach ($this->addressField as $address) {
        $existing_address = $address->value();
        if ($this->compareAddresses($existing_address, $given_address)) {
          return $address->value()['address_number'];
        }
      }
    }
    else {
      if ($this->compareAddresses($this->addressField->value(), $given_address)) {
        return $this->addressField->address_number->value();
      }
    }
    return 0;
  }

  /**
   * Compare two address arrays, case-insensitively, for equality.
   */
  protected function compareAddresses($address1, $address2) {
    $field_names = [
      'postcode',
      'town',
      'address_line_1',
      'address_line_2',
      'address_line_3',
    ];
    foreach ($field_names as $field_name) {
      $value1 = strtolower(trim($address1[$field_name]));
      $value2 = strtolower(trim($address2[$field_name]));
      if ($value1 != $value2) {
        return FALSE;
      }
    }
    return TRUE;
  }

  /**
   * Load all data into account from CARE.
   */
  public function fetchFromCare() {
    if ($this->membershipNumber) {
      $start_time = microtime(TRUE);
      $this->fetchBasicDetails();
      $this->fetchCommunicationDetails();
      $this->fetchAddressDetails();
      $this->fetchMembershipDetails();
      $refresh_time = microtime(TRUE) - $start_time;
      watchdog('care_user', 'CARE took @secs s for @user in fetchFromCare().', [
        '@user' => $this->account->raw()->name,
        '@secs' => round($refresh_time, 3, PHP_ROUND_HALF_UP),
      ], WATCHDOG_INFO);
    }
  }

  /**
   * Load basic details from CARE.
   */
  public function fetchBasicDetails() {
    if ($this->membershipNumber && ($this->nameField || $this->dobField)) {
      watchdog('care_user', 'fetchBasicDetails for %user (%membership)', [
        '%user' => $this->account->raw()->name,
        '%membership' => $this->membershipNumber,
      ]);
      $data = [
        'ContactNumber' => $this->membershipNumber,
      ];
      // Use FindContacts in case the membership has been merged into another.
      $care_xml_data = care_call_method('FindContacts', $data);
      if (isset($care_xml_data->ErrorMessage)) {
        watchdog('care_user', 'CARE error fetchBasicDetails: @error', [
          '@error' => (string) $care_xml_data->ErrorMessage,
        ], WATCHDOG_ERROR);
      }
      else {
        // Update the membership number if changed due to a merge.
        if ($care_xml_data->DataRow->ContactNumber != $this->membershipNumber) {
          $this->membershipNumberField->number = $care_xml_data->DataRow->ContactNumber;
        }
        // Update the name field.
        if ($this->nameField) {
          $data = [
            'title' => (string) $care_xml_data->DataRow->Title,
            'forenames' => (string) $care_xml_data->DataRow->Forenames,
            'surname' => (string) $care_xml_data->DataRow->Surname,
            'local_unsent_changes' => CARE_FIELDS_UNCHANGED,
          ];
          $this->nameField->set($data);
        }
        // Update the date of birth field.
        if ($this->dobField) {
          $date_string = (string) $care_xml_data->DataRow->DateOfBirth;
          if ($date_string) {
            $date_array = explode("/", $date_string);
            $data = [
              'day' => $date_array[0],
              'month' => $date_array[1],
              'year' => $date_array[2],
              'local_unsent_changes' => CARE_FIELDS_UNCHANGED,
            ];
            $this->dobField->set($data);
          }
        }
      }
      // Save fields without saving the parent user.
      $this->saveFieldsToDrupal();
    }
  }

  /**
   * Load email and telephone data from CARE.
   */
  public function fetchCommunicationDetails() {
    if ($this->membershipNumber && ($this->emailField || $this->telephoneField)) {
      watchdog('care_user', 'fetchCommunicationDetails for %user (%membership)', [
        '%user' => $this->account->raw()->name,
        '%membership' => $this->membershipNumber,
      ]);
      $start_time = microtime(TRUE);
      $data = [
        'ContactNumber' => $this->membershipNumber,
      ];
      $typedata = [
        'pSelectDataType' => 'xcdtContactCommsNumbers',
      ];
      $care_xml_data = care_call_method('SelectContactData', $data, $typedata);
      if (isset($care_xml_data->ErrorMessage)) {
        watchdog('care_user', 'CARE error fetchCommunicationDetails: @error', [
          '@error' => (string) $care_xml_data->ErrorMessage,
        ], WATCHDOG_ERROR);
      }
      else {
        // Email field.
        if ($this->emailField) {
          $field_info = field_info_field(variable_get('care_user_email_field', ''));
          $multiple_value_field = ($field_info['cardinality'] == '-1');
          $this->emailField->set(NULL);
          // Get address data from CARE result.
          $addresses = [];
          foreach ($care_xml_data as $care_address) {
            $valid_to = (string) $care_address->ValidTo;
            $valid_in_time = TRUE;
            if ($valid_to) {
              $valid_to_datetime = date_create_from_format('d/m/Y', $valid_to);
              $midnight_datetime = date_create('midnight tomorrow');
              if ($valid_to_datetime < $midnight_datetime) {
                $valid_in_time = FALSE;
              }
            }
            if ($valid_in_time && $care_address->Email == 'Y' && $care_address->IsActive == 'Yes') {
              $is_default = ((string) $care_address->DeviceDefault == 'Yes' ? 1 : 0);
              $comm_number = (string) $care_address->CommunicationNumber;
              $index = ($is_default ? -$comm_number : $comm_number);
              $addresses[$index] = [
                'communication_number' => $comm_number,
                'device_code' => (string) $care_address->DeviceCode,
                'email' => (string) $care_address->PhoneNumber,
                'local_unsent_changes' => CARE_FIELDS_UNCHANGED,
                'device_default' => $is_default,
              ];
            }
          }
          // Sort addresses by default, then communication number.
          ksort($addresses);
          $addresses = array_values($addresses);
          // Set field data.
          if ($multiple_value_field) {
            $this->emailField->set($addresses);
          }
          else {
            foreach ($addresses as $i => $address) {
              // Default or last (most recent) value.
              if ($address['device_default'] || ($i == count($addresses) - 1)) {
                $this->emailField->set($address);
                break;
              }
            }
          }
        }
        // Telephone field.
        if ($this->telephoneField) {
          $field_info = field_info_field(variable_get('care_user_telephone_field', ''));
          $multiple_value_field = ($field_info['cardinality'] == '-1');
          $this->telephoneField->set(NULL);
          // Get address data from CARE result.
          $addresses = [];
          foreach ($care_xml_data as $care_address) {
            $valid_to = date_create_from_format('d/m/Y', (string) $care_address->ValidTo);
            $address_codes = [
              "MB",
              "TH",
              "TE",
              "S",
            ];
            if ($care_address->IsActive == 'Yes' && (!$valid_to || $valid_to >= date_create()) && in_array($care_address->DeviceCode, $address_codes)) {
              $is_default = ((string) $care_address->DeviceDefault == 'Yes' ? 1 : 0);
              $comm_number = (string) $care_address->CommunicationNumber;
              $index = ($is_default ? -$comm_number : $comm_number);
              $addresses[$index] = [
                'communication_number' => $comm_number,
                'device_code' => (string) $care_address->DeviceCode,
                'telephone' => (string) $care_address->PhoneNumber,
                'local_unsent_changes' => CARE_FIELDS_UNCHANGED,
                'device_default' => $is_default,
              ];
            }
          }
          // Sort addresses by default, then communication number.
          ksort($addresses);
          $addresses = array_values($addresses);
          // Set field data.
          if ($multiple_value_field) {
            $this->telephoneField->set($addresses);
          }
          else {
            foreach ($addresses as $i => $address) {
              // Default or last value.
              if ($address['device_default'] || ($i == count($addresses) - 1)) {
                $this->telephoneField->set($address);
                break;
              }
            }
          }
        }
        $refresh_time = microtime(TRUE) - $start_time;
        watchdog('care_user', 'CARE took @secs s for @user in fetchCommunicationDetails().', [
          '@user' => $this->account->raw()->name,
          '@secs' => round($refresh_time, 3, PHP_ROUND_HALF_UP),
        ], WATCHDOG_INFO);
        // Save fields without saving the parent user.
        $this->saveFieldsToDrupal();
      }
    }
  }

  /**
   * Load postal address details from CARE.
   */
  public function fetchAddressDetails() {
    if ($this->membershipNumber && $this->addressField) {
      watchdog('care_user', 'fetchAddressDetails for %user (%membership)', [
        '%user' => $this->account->raw()->name,
        '%membership' => $this->membershipNumber,
      ]);
      $start_time = microtime(TRUE);
      $field_info = field_info_field(variable_get('care_user_address_field', ''));
      $multiple_value_field = ($field_info['cardinality'] == '-1');
      $i = 0;
      $data = [
        'ContactNumber' => $this->membershipNumber,
      ];
      $typedata = [
        'pSelectDataType' => 'xcdtContactAddresses',
      ];
      $care_xml_data = care_call_method('SelectContactData', $data, $typedata);
      if (isset($care_xml_data->ErrorMessage)) {
        watchdog('care_user', 'CARE error fetchAddressDetails: @error', [
          '@error' => (string) $care_xml_data->ErrorMessage,
        ], WATCHDOG_ERROR);
      }
      else {
        $addresses = [];
        foreach ($care_xml_data as $care_address) {
          $valid_to = (string) $care_address->ValidTo;
          $valid_in_time = TRUE;
          if ($valid_to) {
            $valid_to_datetime = date_create_from_format('d/m/Y', $valid_to);
            $midnight_datetime = date_create('midnight tomorrow');
            if ($valid_to_datetime < $midnight_datetime) {
              $valid_in_time = FALSE;
            }
          }
          $dummy_address = ((string) $care_address->Town == 'TOWN-REQUIRD' || (string) $care_address->Town == 'unknown');
          $contact_address = ((string) $care_address->AddressType == 'C');
          $historical = ((string) $care_address->Historical == 'Y');
          if (!$historical && $valid_in_time && $contact_address && !$dummy_address) {
            $addresses[] = [
              'address_number' => (string) $care_address->AddressNumber,
              'address_line_1' => (string) $care_address->AddressLine1,
              'address_line_2' => (string) $care_address->AddressLine2,
              'address_line_3' => (string) $care_address->AddressLine3,
              'town' => (string) $care_address->Town,
              'county' => (string) $care_address->County,
              'country_code' => (string) $care_address->CountryCode,
              'postcode' => (string) $care_address->Postcode,
              'default' => (string) $care_address->Default,
              'local_unsent_changes' => CARE_FIELDS_UNCHANGED,
              'address_index' => 0,
            ];
          }
        }
        usort($addresses, function ($a, $b) {
          if ($a['default'] == 'Yes') {
            return -1;
          }
          return ($a['address_number'] < $b['address_number'] ? -1 : 1);
        });
        $this->addressField->set(NULL);
        // Set field data.
        if ($multiple_value_field) {
          // All values.
          foreach ($addresses as $i => $address) {
            $this->addressField[$i]->set($address);
          }
        }
        else {
          foreach ($addresses as $i => $address) {
            // Default or last value.
            if ($address['default'] || ($i == count($addresses) - 1)) {
              $this->addressField->set($address);
              break;
            }
          }
        }
      }
      $refresh_time = microtime(TRUE) - $start_time;
      watchdog('care_user', 'CARE took @secs s for @user in fetchAddressDetails().', [
        '@user' => $this->account->raw()->name,
        '@secs' => round($refresh_time, 3, PHP_ROUND_HALF_UP),
      ], WATCHDOG_INFO);
      // Save fields without saving the parent user.
      $this->saveFieldsToDrupal();
    }
  }

  /**
   * Load membership data from CARE.
   */
  public function fetchMembershipDetails() {
    if ($this->membershipNumber && $this->membershipsField) {
      watchdog('care_user', 'fetchMembershipDetails for %user (%membership)', [
        '%user' => $this->account->raw()->name,
        '%membership' => $this->membershipNumber,
      ]);
      $start_time = microtime(TRUE);
      $data = [
        'ContactNumber' => $this->membershipNumber,
      ];
      $care_xml_data = care_call_method('FindMembers', $data);
      if (!$care_xml_data->ErrorMessage) {
        $this->membershipsField->set(NULL);
        $field_info = field_info_field(variable_get('care_user_memberships_field', ''));
        $multiple_values = ($field_info['cardinality'] == '-1');
        $i = 0;
        $memberships_data = [];
        foreach ($care_xml_data as $member) {
          // Only include non-cancelled membership records.
          $not_cancelled = (string) $member->CancellationReason == '';
          if ($not_cancelled) {
            $renewaldate = DateTime::createFromFormat('d/m/Y', (string) $member->RenewalDate);
            if (!$renewaldate) {
              watchdog('care_user', 'Constructon DateTime from d/m/Y RenewalDate %datestring failed', ['%datestring' => (string) $member->RenewalDate], WATCHDOG_ERROR);
            }
            $membership_data = [
              'membership_type' => (string) $member->MembershipType,
              'membership_number' => (int) $member->MembershipNumber,
              'payment_plan_number' => (int) $member->PaymentPlanNumber,
              'renewal_date' => $renewaldate ? $renewaldate->format('Y-m-d') : '',
              'balance' => (float) $member->Balance,
              'payment_method' => '',
              'refreshed' => date('Y-m-d H:i:s'),
            ];
            // Check for DD payments.
            $data = [
              'PaymentPlanNumber' => (int) $member->PaymentPlanNumber,
            ];
            $care_dd_xml_data = care_call_method('FindPaymentPlans', $data);
            if (!$care_dd_xml_data->ErrorMessage) {
              $plan = $care_dd_xml_data->DataRow;
              $membership_data['payment_method'] = (string) $plan->PaymentMethod;
            }
            $memberships_data[] = $membership_data;
          }
        }
        foreach ($memberships_data as $data) {
          if ($multiple_values) {
            // Add all addresses if multi-value field.
            $this->membershipsField[$i]->set($data);
            $i++;
          }
          else {
            // Use first membership if single-value field.
            $this->membershipsField->set($data);
            break;
          }
        }
      }
      $refresh_time = microtime(TRUE) - $start_time;
      watchdog('care_user', 'CARE took @secs s for @user in fetchMembershipDetails().', [
        '@user' => $this->account->raw()->name,
        '@secs' => round($refresh_time, 3, PHP_ROUND_HALF_UP),
      ], WATCHDOG_INFO);
      // Save fields without saving the parent user.
      $this->saveFieldsToDrupal();
    }
  }

  /**
   * Save all data to CARE.
   */
  public function saveToCare() {
    $start_time = microtime(TRUE);
    $success = $this->saveBasicDetails();
    $success = $this->saveAddresses() && $success;
    $success = $this->saveTelephones() && $success;
    $success = $this->saveEmails() && $success;
    $refresh_time = microtime(TRUE) - $start_time;
    watchdog('care_user', 'CARE took @secs s for @user (UID=@uid) in saveToCare().', [
      '@user' => $this->account->raw()->name,
      '@uid' => $this->account->raw()->uid,
      '@secs' => round($refresh_time, 3, PHP_ROUND_HALF_UP),
    ], WATCHDOG_INFO);
    return $success;
  }

  /**
   * Save basic data to CARE.
   */
  public function saveBasicDetails() {
    watchdog('care_user', 'saveBasicDetails for %user (%membership)', [
      '%user' => $this->account->raw()->name,
      '%membership' => $this->membershipNumber,
    ]);
    $success = TRUE;
    if ($this->membershipNumber) {
      // Update existing contact.
      $name_changed = ($this->nameField && $this->nameField->value() && $this->nameField->local_unsent_changes->value());
      $dob_changed = ($this->dobField && $this->dobField->value() && $this->dobField->local_unsent_changes->value());
      if ($name_changed || $dob_changed) {
        $data = [
          'ContactNumber' => $this->membershipNumber,
        ];
        if ($name_changed) {
          $data += [
            'Title' => $this->nameField->title->value(),
            'Forenames' => $this->nameField->forenames->value(),
            'Surname' => $this->nameField->surname->value(),
          ];
        }
        if ($dob_changed) {
          $day = $this->dobField->day->value();
          $month = $this->dobField->month->value();
          $year = $this->dobField->year->value();
          $data += [
            'DateOfBirth' => "$year-$month-$day",
          ];
        }
        $resultxml = care_call_method('UpdateContact', $data);
        if (isset($resultxml->ErrorMessage)) {
          $success = FALSE;
        }
        else {
          if ($name_changed) {
            $this->nameField->local_unsent_changes = CARE_FIELDS_UNCHANGED;
          }
          if ($dob_changed) {
            $this->dobField->local_unsent_changes = CARE_FIELDS_UNCHANGED;
          }
          // Save fields without saving the parent user.
          $this->saveFieldsToDrupal();
          watchdog('care_user', 'Updated existing user UID=%uid ContactNumber=%contact.', [
            '%uid' => $this->account->raw()->uid,
            '%contact' => $this->membershipNumber,
          ]);
        }
      }
    }
    else {
      // Min reqs are a completed name field and a membership number field.
      // ToDo: use first address field, if it has a value, and update it
      // with AddressNumber?
      if ($this->nameField && $this->membershipNumberField && $this->nameField->value()) {
        // Add new must include Surname, Address, Town, Country and Source.
        // ToDo: use actual address and email if known.
        if ($this->nameField->type() == 'care_name') {
          $title = $this->nameField->title->value();
          $forenames = $this->nameField->forenames->value();
          $surname = $this->nameField->surname->value();
        }
        else {
          $title = '';
          $forenames = '';
          $surname = $this->nameField->value();
        }
        $data = [
          'Title' => $title,
          'Forenames' => $forenames,
          'Surname' => $surname,
          'Address' => 'unknown',
          'Town' => 'unknown',
          'Country' => 'UK',
          'Source' => 'OI',
        ];
        if ($this->dobField && $this->dobField->value()) {
          $day = $this->dobField->day->value();
          $month = $this->dobField->month->value();
          $year = $this->dobField->year->value();
          $data += [
            'DateOfBirth' => "$year-$month-$day",
          ];
        }
        $resultxml = care_call_method('AddContact', $data);
        if (isset($resultxml->ErrorMessage)) {
          $success = FALSE;
        }
        else {
          $this->membershipNumber = (string) $resultxml->ContactNumber;
          $this->nameField->local_unsent_changes = CARE_FIELDS_UNCHANGED;
          if ($this->dobField && $this->dobField->value()) {
            $this->dobField->local_unsent_changes = CARE_FIELDS_UNCHANGED;
          }
          $this->membershipNumberField->set([
            'number' => $this->membershipNumber,
            'local_unsent_changes' => CARE_FIELDS_UNCHANGED,
          ]);
          // Save fields without saving the parent user.
          $this->saveFieldsToDrupal();
          watchdog('care_user', 'New CARE ContactNumber = %contact.', [
            '%uid' => $this->account->raw()->uid,
            '%contact' => $this->membershipNumber,
          ]);
        }
      }
    }
    return $success;
  }

  /**
   * Save postal address data to CARE.
   */
  public function saveAddresses() {
    watchdog('care_user', 'saveAddresses for %user (%membership)', [
      '%user' => $this->account->raw()->name,
      '%membership' => $this->membershipNumber,
    ]);
    $success = TRUE;
    if ($this->membershipNumber && $this->addressField) {
      $field_info = field_info_field(variable_get('care_user_address_field', ''));
      $single_value_field = ($field_info['cardinality'] == 1);
      if ($single_value_field) {
        $success = $this->saveAddress($this->addressField, TRUE);
      }
      else {
        foreach ($this->addressField as $index => $address_field) {
          $success = $this->saveAddress($address_field, (($index == 0) || ($index == null) ? TRUE : FALSE)) && $success;
        }
      }
      // Save fields without saving the parent user.
      $this->saveFieldsToDrupal();
    }
    return $success;
  }

  /**
   * Save email data to CARE.
   */
  public function saveEmails() {
    watchdog('care_user', 'saveEmails for %user (%membership)', [
      '%user' => $this->account->raw()->name,
      '%membership' => $this->membershipNumber,
    ]);
    $success = TRUE;
    if ($this->membershipNumber && $this->emailField) {
      $field_info = field_info_field(variable_get('care_user_email_field', ''));
      $single_value_field = ($field_info['cardinality'] == 1);
      if ($single_value_field) {
        $success = $this->saveEmail($this->emailField, TRUE);
      }
      else {
        foreach ($this->emailField as $index => $email_field) {
          $success = $this->saveEmail($email_field, ($index == 0)) && $success;
        }
      }
      // Save fields without saving the parent user.
      $this->saveFieldsToDrupal();
    }
    return $success;
  }

  /**
   * Save telephone data to CARE.
   */
  public function saveTelephones() {
    watchdog('care_user', 'saveTelephones for %user (%membership)', [
      '%user' => $this->account->raw()->name,
      '%membership' => $this->membershipNumber,
    ]);
    $success = TRUE;
    if ($this->membershipNumber && $this->telephoneField && $this->telephoneField->value()) {
      $field_info = field_info_field(variable_get('care_user_telephone_field', ''));
      $single_value_field = ($field_info['cardinality'] == 1);
      if ($single_value_field) {
        $success = $this->saveTelephone($this->telephoneField, TRUE);
      }
      else {
        foreach ($this->telephoneField as $index => $telephone_field) {
          $success = $this->saveTelephone($telephone_field, ($index == 0)) && $success;
        }
      }
      // Save fields without saving the parent user.
      $this->saveFieldsToDrupal();
    }
    return $success;
  }

  /**
   * Save single postal address field to CARE.
   */
  protected function saveAddress(EntityMetadataWrapper $address_field, $default = FALSE) {
    $success = FALSE;
    $address_number = $address_field->address_number->value();
    $full_address = $address_field->address_line_1->value() . "\n";
    $full_address .= $address_field->address_line_2->value() . "\n";
    $full_address .= $address_field->address_line_3->value();
    $data = [
      'ContactNumber' => $this->membershipNumber,
      'Address' => $full_address,
      'Town' => $address_field->town->value(),
      'County' => $address_field->county->value(),
      'Postcode' => $address_field->postcode->value(),
      'Country' => $address_field->country_code->value(),
      'Default' => ($default ? 'Y' : 'N'),
    ];
    $old_data = [
      'ContactNumber' => $this->membershipNumber,
      'Default' => 'N',
      'ValidTo' => date('d/m/Y H:i:s'),
    ];
    if ($address_number) {
      $data['AddressNumber'] = $address_number;
      $old_data['AddressNumber'] = $address_number;
      if ($address_field->local_unsent_changes->value() == CARE_FIELDS_CHANGED) {
        // Update existing address.
        if (variable_get('care_user_keep_history', TRUE)) {
          // Invalidate old address record, without changing it.
          $result_xml = care_call_method('UpdateAddress', $old_data);
          if (!isset($result_xml->ErrorMessage)) {
            // Create new address record and update AddressNumber.
            unset($data['AddressNumber']);
            $result_xml = care_call_method('AddAddress', $data);
            if (!isset($result_xml->ErrorMessage)) {
              $address_field->address_number->set((string) $result_xml->AddressNumber);
              $success = TRUE;
            }
          }
        }
        else {
          // Update existing address.
          $result_xml = care_call_method('UpdateAddress', $data);
          if (!isset($result_xml->ErrorMessage)) {
            $success = TRUE;
          }
        }
      }
      elseif ($address_field->local_unsent_changes->value() == CARE_FIELDS_DELETED) {
        // Delete existing address.
        $result_xml = care_call_method('UpdateAddress', $old_data);
        if (!isset($result_xml->ErrorMessage)) {
          $address_field->set(NULL);
          $success = TRUE;
        }
      }
      else {
        // No change.
        $success = TRUE;
      }
    }
    else {
      // Add new address and store AddressNumber if successful.
      $result_xml = care_call_method('AddAddress', $data);
      if (!isset($result_xml->ErrorMessage)) {
        $address_field->address_number->set((string) $result_xml->AddressNumber);
        $success = TRUE;
      }
    }
    if ($success && $address_field->value()) {
      // Record that we've saved changes to CARE.
      $address_field->local_unsent_changes = CARE_FIELDS_UNCHANGED;
    }
    return $success;
  }

  /**
   * Save single email address to CARE.
   */
  protected function saveEmail(EntityMetadataWrapper $email_field, $default = FALSE) {
    $success = FALSE;
    $communication_number = $email_field->communication_number->value();
    $data = [
      'ContactNumber' => $this->membershipNumber,
      'OldContactNumber' => $this->membershipNumber,
      'Number' => $email_field->email->value(),
      'Device' => $email_field->device_code->value(),
      'DeviceDefault' => ($default ? 'Y' : 'N'),
    ];
    $old_data = [
      'OldContactNumber' => $this->membershipNumber,
      'DeviceDefault' => 'N',
      'ValidTo' => date('d/m/Y H:i:s'),
    ];
    if ($communication_number) {
      $data['CommunicationNumber'] = $communication_number;
      $old_data['CommunicationNumber'] = $communication_number;
      if ($email_field->local_unsent_changes->value() == CARE_FIELDS_CHANGED) {
        // Update existing email.
        if (variable_get('care_user_keep_history', TRUE)) {
          // Invalidate old email record.
          $resultxml = care_call_method('UpdateCommunicationsNumber', $old_data);
          if (!isset($resultxml->ErrorMessage)) {
            // Create new email record and update CommunicationsNumber.
            unset($data['CommunicationNumber']);
            $resultxml = care_call_method('AddCommunicationsNumber', $data);
            if (!isset($resultxml->ErrorMessage)) {
              $email_field->communication_number->set((string) $resultxml->CommunicationNumber);
              $success = TRUE;
            }
          }
        }
        else {
          // Update email record.
          $resultxml = care_call_method('UpdateCommunicationsNumber', $data);
          if (!isset($resultxml->ErrorMessage)) {
            $success = TRUE;
          }
        }
      }
      elseif ($email_field->local_unsent_changes->value() == CARE_FIELDS_DELETED) {
        // Invalidate email record.
        $resultxml = care_call_method('UpdateCommunicationsNumber', $old_data);
        if (!isset($resultxml->ErrorMessage)) {
          $email_field->set(NULL);
          $success = TRUE;
        }
      }
      elseif ($email_field->value() && $default) {
        watchdog('care_user', 'Set DeviceDefault for %e', [
          '%e' => $email_field->email->value(),
        ]);
        $non_default_data = [
          'OldContactNumber' => $this->membershipNumber,
          'CommunicationNumber' => $communication_number,
          'DeviceDefault' => 'Y',
        ];
        $resultxml = care_call_method('UpdateCommunicationsNumber', $non_default_data);
        if (!isset($resultxml->ErrorMessage)) {
          $success = TRUE;
        }
      }
      elseif ($email_field->value() && !$default) {
        $value = $email_field->value();
        if ($value['device_default']) {
          watchdog('care_user', 'Unset DeviceDefault for %e', [
            '%e' => $email_field->email->value(),
          ]);
          $non_default_data = [
            'OldContactNumber' => $this->membershipNumber,
            'CommunicationNumber' => $communication_number,
            'DeviceDefault' => 'N',
          ];
          $resultxml = care_call_method('UpdateCommunicationsNumber', $non_default_data);
          if (!isset($resultxml->ErrorMessage)) {
            $value['device_default'] = 0;
            $email_field->set($value);
            $success = TRUE;
          }
        }
        else {
          $success = TRUE;
        }
      }
      else {
        watchdog('care_user', 'Nothing to do for %e', [
          '%e' => $email_field->email->value(),
        ]);
        $success = TRUE;
      }
    }
    else {
      // Add new email record and store CommunicationsNumber if successful.
      $resultxml = care_call_method('AddCommunicationsNumber', $data);
      if (!isset($resultxml->ErrorMessage)) {
        $email_field->communication_number->set((string) $resultxml->CommunicationNumber);
        $success = TRUE;
      }
    }
    if ($success && $email_field->value()) {
      // Record that we've saved changes to CARE.
      $email_field->local_unsent_changes = CARE_FIELDS_UNCHANGED;
    }
    return $success;
  }

  /**
   * Save single telephone number to CARE.
   */
  protected function saveTelephone(EntityMetadataWrapper $telephone_field, $default = FALSE) {
    $success = FALSE;
    $communication_number = $telephone_field->communication_number->value();
    $data = [
      'ContactNumber' => $this->membershipNumber,
      'OldContactNumber' => $this->membershipNumber,
      'Number' => $telephone_field->telephone->value(),
      'Device' => $telephone_field->device_code->value(),
      'DeviceDefault' => ($default ? 'Y' : 'N'),
    ];
    $old_data = [
      'OldContactNumber' => $this->membershipNumber,
      'DeviceDefault' => 'N',
      'ValidTo' => date('d/m/Y H:i:s'),
    ];
    if ($communication_number) {
      $data['CommunicationNumber'] = $communication_number;
      $old_data['CommunicationNumber'] = $communication_number;
      if ($telephone_field->local_unsent_changes->value() == CARE_FIELDS_CHANGED) {
        // Update existing telephone.
        if (variable_get('care_user_keep_history', TRUE)) {
          // Invalidate old telephone record.
          $resultxml = care_call_method('UpdateCommunicationsNumber', $old_data);
          if (!isset($resultxml->ErrorMessage)) {
            // Create new telephone record and update CommunicationsNumber.
            unset($data['CommunicationNumber']);
            $resultxml = care_call_method('AddCommunicationsNumber', $data);
            if (!isset($resultxml->ErrorMessage)) {
              $telephone_field->communication_number->set((string) $resultxml->CommunicationNumber);
              $success = TRUE;
            }
          }
        }
        else {
          // Update telephone record.
          $resultxml = care_call_method('UpdateCommunicationsNumber', $data);
          if (!isset($resultxml->ErrorMessage)) {
            $success = TRUE;
          }
        }
      }
      elseif ($telephone_field->local_unsent_changes->value() == CARE_FIELDS_DELETED) {
        // Delete existing address.
        $resultxml = care_call_method('UpdateCommunicationsNumber', $old_data);
        if (!isset($resultxml->ErrorMessage)) {
          $telephone_field->set(NULL);
          $success = TRUE;
        }
      }
      else {
        // Nothing to do.
        $success = TRUE;
      }
    }
    else {
      // Add new telephone and store CommunicationNumber if successful.
      $resultxml = care_call_method('AddCommunicationsNumber', $data);
      if (!isset($resultxml->ErrorMessage)) {
        $telephone_field->communication_number->set((string) $resultxml->CommunicationNumber);
        $success = TRUE;
      }
    }
    if ($success && $telephone_field->value()) {
      // Record that we've saved changes to CARE.
      $telephone_field->local_unsent_changes = CARE_FIELDS_UNCHANGED;
    }
    return $success;
  }

  /**
   * Save field data to Drupal database, unless we have the anonymous user.
   */
  protected function saveFieldsToDrupal() {
    if ($this->account->raw()->uid != 0) {
      field_attach_update('user', $this->account->raw());
    }
  }

}
