<?php
/**
 * @file
 * Define the CareUser class.
 */

class CareUser {

  private $account;

  private $waccount;

  private $membershipNumber;

  private $membershipNumberField;

  private $nameField;

  private $dobField;

  private $addressField;

  private $emailField;

  private $telephoneField;

  private $membershipsField;

  /**
   * Construct a CareUser object from a Drupal account.
   */
  public function __construct($account) {
    $this->account = $account;
    $this->waccount = entity_metadata_wrapper('user', $account);
    $membership_number_field_name = variable_get('care_user_membership_number_field', '');
    if ($membership_number_field_name and isset($this->waccount->{$membership_number_field_name})) {
      $this->membershipNumberField = $this->waccount->{$membership_number_field_name};
      if ($this->waccount->{$membership_number_field_name}->value()) {
        $this->membershipNumber = $this->membershipNumberField->number->value();
      }
    }
    $name_field_name = variable_get('care_user_name_field', '');
    if ($name_field_name and isset($this->waccount->{$name_field_name})) {
      $this->nameField = $this->waccount->{$name_field_name};
    }
    $dob_field_name = variable_get('care_user_dob_field', '');
    if ($dob_field_name and isset($this->waccount->{$dob_field_name})) {
      $this->dobField = $this->waccount->{$dob_field_name};
    }
    $address_field_name = variable_get('care_user_address_field', '');
    if ($address_field_name and isset($this->waccount->$address_field_name)) {
      $this->addressField = $this->waccount->{$address_field_name};
    }
    $email_field_name = variable_get('care_user_email_field', '');
    if ($email_field_name and isset($this->waccount->$email_field_name)) {
      $this->emailField = $this->waccount->{$email_field_name};
    }
    $telephone_field_name = variable_get('care_user_telephone_field', '');
    if ($telephone_field_name and isset($this->waccount->$telephone_field_name)) {
      $this->telephoneField = $this->waccount->{$telephone_field_name};
    }
    $memberships_field_name = variable_get('care_user_memberships_field', '');
    if ($memberships_field_name and isset($this->waccount->$memberships_field_name)) {
      $this->membershipsField = $this->waccount->{$memberships_field_name};
    }
  }

  /**
   * Return the account's CARE membership number, if any.
   */
  public function getMembershipNumber() {
    return $this->membershipNumber;
  }

  /**
   * Return the email field as a wrapped entity.
   */
  public function getEmailField() {
    return $this->emailField;
  }

  /**
   * Return the memberships field as a wrapped entity.
   */
  public function getMembershipsField() {
    return $this->membershipsField;
  }

  /**
   * Load all data into account from CARE.
   */
  public function fetchFromCare() {
    $this->fetchBasicDetails();
    $this->fetchCommunicationDetails();
    $this->fetchAddressDetails();
    $this->fetchMembershipDetails();
  }

  /**
   * Load basic details from CARE.
   */
  public function fetchBasicDetails() {
    if ($this->membershipNumber and ($this->nameField or $this->dobField)) {
      $start_time = microtime(TRUE);
      $data = array(
        'ContactNumber' => $this->membershipNumber);
      $typedata = array(
        'pSelectDataType' => 'xcdtContactInformation');
      $care_xml_data = care_call_method('SelectContactData', $data, $typedata);
      if (isset($care_xml_data->ErrorMessage)) {
        drupal_set_message($care_xml_data->ErrorMessage);
      }
      else {
        if ($this->nameField) {
          $data = array(
            'title' => (string) $care_xml_data->DataRow->Title,
            'forenames' => (string) $care_xml_data->DataRow->Forenames,
            'surname' => (string) $care_xml_data->DataRow->Surname,
            'local_unsent_changes' => 0);
          $this->nameField->set($data);
        }
        if ($this->dobField) {
          $date_string = (string) $care_xml_data->DataRow->DateOfBirth;
          $date_array = explode("/", $date_string);
          $data = array(
            'day' => $date_array[0],
            'month' => $date_array[1],
            'year' => $date_array[2],
            'local_unsent_changes' => 0);
          $this->dobField->set($data);
        }
      }
      $refresh_time = microtime(TRUE) - $start_time;
      watchdog('care_user', 'CARE took @secs s for @user in fetchBasicDetails().', array(
        '@user' => $this->account->name,
        '@secs' => round($refresh_time, 3, PHP_ROUND_HALF_UP)), WATCHDOG_INFO);
      // Save fields without saving the parent user.
      field_attach_update('user', $this->account);
    }
  }

  /**
   * Load email and telephone data from CARE.
   */
  public function fetchCommunicationDetails() {
    if ($this->membershipNumber and ($this->emailField or $this->telephoneField)) {
      $start_time = microtime(TRUE);
      $data = array(
        'ContactNumber' => $this->membershipNumber);
      $typedata = array(
        'pSelectDataType' => 'xcdtContactCommsNumbers');
      $care_xml_data = care_call_method('SelectContactData', $data, $typedata);
      if (isset($care_xml_data->ErrorMessage)) {
        drupal_set_message($care_xml_data->ErrorMessage);
      }
      else {
        // Email field.
        if ($this->emailField) {
          $field_info = field_info_field(variable_get('care_user_email_field', ''));
          $multiple_value_field = ($field_info['cardinality'] == '-1');
          $this->emailField->set(NULL);
          // Get address data from CARE result.
          $addresses = array();
          foreach ($care_xml_data as $care_address) {
            // dpm($care_address->asXML());
            $valid_to = (string) $care_address->ValidTo;
            $valid_in_time = TRUE;
            if ($valid_to) {
              $valid_to_datetime = date_create_from_format('d/m/Y', $valid_to);
              $midnight_datetime = date_create('midnight tomorrow');
              if ($valid_to_datetime < $midnight_datetime) {
                $valid_in_time = FALSE;
              }
            }
            if ($valid_in_time and $care_address->Email == 'Y' and $care_address->IsActive == 'Yes') {
              $addresses[] = array(
                'communication_number' => (string) $care_address->CommunicationNumber,
                'device_code' => (string) $care_address->DeviceCode,
                'email' => (string) $care_address->Number,
                'local_unsent_changes' => 0,
                'DeviceDefault' => (string) $care_address->DeviceDefault);
            }
          }
          // Sort addresses by communication number.
          usort($addresses, function ($a, $b) {
            if ($a['DeviceDefault'] == 'Yes') {
              return -1;
            }
            return ($a['communication_number'] < $b['communication_number'] ? 1 : -1);
          });
          // Set field data.
          if ($multiple_value_field) {
            // All values.
            foreach ($addresses as $i => $address) {
              $this->emailField[$i]->set($address);
            }
          }
          else {
            foreach ($addresses as $i => $address) {
              // Default or last value.
              if ($address['DeviceDefault'] or ($i == count($addresses) - 1)) {
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
          $addresses = array();
          foreach ($care_xml_data as $care_address) {
            $valid_to = date_create_from_format('d/m/Y', (string) $care_address->ValidTo);
            if ((!$valid_to or $valid_to >= date_create()) and in_array($care_address->DeviceCode, array(
              "MB",
              "TH",
              "TE",
              "S")) and $care_address->IsActive == 'Yes') {
              $addresses[] = array(
                'communication_number' => (string) $care_address->CommunicationNumber,
                'device_code' => (string) $care_address->DeviceCode,
                'telephone' => (string) $care_address->Number,
                'local_unsent_changes' => 0,
                'DeviceDefault' => (string) $care_address->DeviceDefault);
            }
          }

          // Sort addresses by communication number.
          usort($addresses, function ($a, $b) {
            if ($a['DeviceDefault'] == 'Yes') {
              return -1;
            }
            return ($a['communication_number'] < $b['communication_number'] ? 1 : -1);
          });
          // Set field data.
          if ($multiple_value_field) {
            // All values.
            foreach ($addresses as $i => $address) {
              $this->telephoneField[$i]->set($address);
            }
          }
          else {
            foreach ($addresses as $i => $address) {
              // Default or last value.
              if ($address['DeviceDefault'] or ($i == count($addresses) - 1)) {
                $this->telephoneField->set($address);
                break;
              }
            }
          }
        }
        $refresh_time = microtime(TRUE) - $start_time;
        watchdog('care_user', 'CARE took @secs s for @user in fetchCommunicationDetails().', array(
          '@user' => $this->account->name,
          '@secs' => round($refresh_time, 3, PHP_ROUND_HALF_UP)), WATCHDOG_INFO);
        // Save fields without saving the parent user.
        field_attach_update('user', $this->account);
      }
    }
  }

  /**
   * Load postal address details from CARE.
   */
  public function fetchAddressDetails() {
    if ($this->membershipNumber and $this->addressField) {
      $start_time = microtime(TRUE);
      $field_info = field_info_field(variable_get('care_user_address_field', ''));
      $multiple_value_field = ($field_info['cardinality'] == '-1');
      $i = 0;
      $data = array(
        'ContactNumber' => $this->membershipNumber);
      $typedata = array(
        'pSelectDataType' => 'xcdtContactAddresses');
      $care_xml_data = care_call_method('SelectContactData', $data, $typedata);
      if (isset($care_xml_data->ErrorMessage)) {
        drupal_set_message($care_xml_data->ErrorMessage);
      }
      else {
        $addresses = array();
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
          $dummy_address = ((string) $care_address->Town == 'TOWN-REQUIRD' or (string) $care_address->Town == 'unknown');
          $contact_address = ((string) $care_address->AddressType == 'C');
          $historical = ((string) $care_address->Historical == 'Y');
          if (!$historical and $valid_in_time and $contact_address and !$dummy_address) {
            $addresses[] = array(
              'address_number' => (string) $care_address->AddressNumber,
              'address_line_1' => (string) $care_address->AddressLine1,
              'address_line_2' => (string) $care_address->AddressLine2,
              'address_line_3' => (string) $care_address->AddressLine3,
              'town' => (string) $care_address->Town,
              'county' => (string) $care_address->County,
              'country_code' => (string) $care_address->CountryCode,
              'postcode' => (string) $care_address->Postcode,
              'default' => (string) $care_address->Default,
              'local_unsent_changes' => 0,
              'address_index' => 0);
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
            if ($address['default'] or ($i == count($addresses) - 1)) {
              $this->addressField->set($address);
              break;
            }
          }
        }
      }
      $refresh_time = microtime(TRUE) - $start_time;
      watchdog('care_user', 'CARE took @secs s for @user in fetchAddressDetails().', array(
        '@user' => $this->account->name,
        '@secs' => round($refresh_time, 3, PHP_ROUND_HALF_UP)), WATCHDOG_INFO);
      // Save fields without saving the parent user.
      field_attach_update('user', $this->account);
    }
  }

  /**
   * Load membership data from CARE.
   */
  public function fetchMembershipDetails() {
    if ($this->membershipNumber and $this->membershipsField) {
      $start_time = microtime(TRUE);
      $data = array(
        'ContactNumber' => $this->membershipNumber);
      $care_xml_data = care_call_method('FindMembers', $data);
      if (!$care_xml_data->ErrorMessage) {
        $this->membershipsField->set(NULL);
        $field_info = field_info_field(variable_get('care_user_memberships_field', ''));
        $multiple_values = ($field_info['cardinality'] == '-1');
        $i = 0;
        $memberships_data = array();
        foreach ($care_xml_data as $member) {
          // Look for organisation membership records with non-zero balance.
          $not_cancelled = (string) $member->CancellationReason == '';
          if ($not_cancelled) {
            $renewaldate = DateTime::createFromFormat('d/m/Y', $member->RenewalDate);
            $membership_data = array(
              'membership_type' => (string) $member->MembershipType,
              'membership_number' => (int) $member->MembershipNumber,
              'payment_plan_number' => (int) $member->PaymentPlanNumber,
              'renewal_date' => $renewaldate->format('Y-m-d'),
              'balance' => (float) $member->Balance,
              'payment_method' => '',
              'refreshed' => date('Y-m-d H:i:s'));
            // Check for DD payments.
            $data = array(
              'PaymentPlanNumber' => (int) $member->PaymentPlanNumber);
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
      watchdog('care_user', 'CARE took @secs s for @user in fetchMembershipDetails().', array(
        '@user' => $this->account->name,
        '@secs' => round($refresh_time, 3, PHP_ROUND_HALF_UP)), WATCHDOG_INFO);
      // Save fields without saving the parent user.
      field_attach_update('user', $this->account);
    }
  }

  /**
   * Save all data to CARE.
   */
  public function saveToCare() {
    $success = $this->saveBasicDetails();
    $success = $this->saveAddresses() && $success;
    $success = $this->saveTelephones() && $success;
    $success = $this->saveEmails() && $success;
    return $success;
  }

  /**
   * Save basic data to CARE.
   */
  public function saveBasicDetails() {
    $success = TRUE;
    if ($this->membershipNumber) {
      // Update existing contact.
      $name_changed = ($this->nameField and $this->nameField->value() and $this->nameField->local_unsent_changes->value());
      $dob_changed = ($this->dobField and $this->dobField->value() and $this->dobField->local_unsent_changes->value());
      if ($name_changed or $dob_changed) {
        $data = array(
          'ContactNumber' => $this->membershipNumber);
        if ($name_changed) {
          $data += array(
            'Title' => $this->nameField->title->value(),
            'Forenames' => $this->nameField->forenames->value(),
            'Surname' => $this->nameField->surname->value());
        }
        if ($dob_changed) {
          $day = $this->dobField->day->value();
          $month = $this->dobField->month->value();
          $year = $this->dobField->year->value();
          $data += array(
            'DateOfBirth' => "$year-$month-$day");
        }
        $resultxml = care_call_method('UpdateContact', $data);
        if (!isset($resultxml->ErrorMessage)) {
          if ($name_changed) {
            $this->nameField->local_unsent_changes->set(0);
          }
          if ($dob_changed) {
            $this->dobField->local_unsent_changes->set(0);
          }
          // Save fields without saving the parent user.
          field_attach_update('user', $this->account);
          watchdog('care_user', 'Updated existing user UID=%uid ContactNumber=%contact.', array(
            '%uid' => $this->account->uid,
            '%contact' => $this->membershipNumber));
        }
        else {
          $success = FALSE;
        }
      }
    }
    else {
      // Min reqs are a completed name field and a membership number field.
      // ToDo: use first address field, if it has a value, and update it
      // with AddressNumber?
      if ($this->nameField and $this->nameField->value() and $this->membershipNumberField) {
        // Add new must include Surname, Address, Town, Country and Source.
        $data = array(
          'Title' => $this->nameField->title->value(),
          'Forenames' => $this->nameField->forenames->value(),
          'Surname' => $this->nameField->surname->value(),
          'Address' => 'unknown',
          'Town' => 'unknown',
          'Country' => 'UK',
          'Source' => 'OI');
        if ($this->dobField and $this->dobField->value()) {
          $day = $this->dobField->day->value();
          $month = $this->dobField->month->value();
          $year = $this->dobField->year->value();
          $data += array(
            'DateOfBirth' => "$year-$month-$day");
        }
        $resultxml = care_call_method('AddContact', $data);
        if (!isset($resultxml->ErrorMessage)) {
          $this->membershipNumber = (string) $resultxml->ContactNumber;
          $this->nameField->local_unsent_changes->set(0);
          if ($this->dobField and $this->dobField->value()) {
            $this->dobField->local_unsent_changes->set(0);
          }
          $this->membershipNumberField->set(array(
            'number' => $this->membershipNumber,
            'local_unsent_changes' => 0));
          // Save fields without saving the parent user.
          field_attach_update('user', $this->account);
          watchdog('care_user', 'Created new user UID=%uid ContactNumber=%contact.', array(
            '%uid' => $this->account->uid,
            '%contact' => $this->membershipNumber));
        }
        else {
          $success = FALSE;
        }
      }
    }
    return $success;
  }

  /**
   * Save postal address data to CARE.
   */
  public function saveAddresses() {
    $success = TRUE;
    if ($this->membershipNumber and $this->addressField and $this->addressField->value()) {
      $field_info = field_info_field(variable_get('care_user_address_field', ''));
      $single_value_field = ($field_info['cardinality'] == 1);
      if ($single_value_field) {
        $success = $this->saveAddress($this->addressField, TRUE);
      }
      else {
        foreach ($this->addressField as $index => $address_field) {
          $success = $this->saveAddress($address_field, ($index == 0)) && $success;
        }
      }
      // Save fields without saving the parent user.
      field_attach_update('user', $this->account);
    }
    else {
      $success = FALSE;
    }
    return $success;
  }

  /**
   * Save email data to CARE.
   */
  public function saveEmails() {
    $success = TRUE;
    if ($this->membershipNumber and $this->emailField and $this->emailField->value()) {
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
      field_attach_update('user', $this->account);
    }
    else {
      $success = FALSE;
    }
    return $success;
  }

  /**
   * Save telephone data to CARE.
   */
  public function saveTelephones() {
    $success = TRUE;
    if ($this->membershipNumber and $this->telephoneField and $this->telephoneField->value()) {
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
      field_attach_update('user', $this->account);
    }
    else {
      $success = FALSE;
    }
    return $success;
  }

  /**
   * Save single postal address field to CARE.
   */
  protected function saveAddress(EntityMetadataWrapper &$address_field, $default = FALSE) {
    $success = FALSE;
    $address_number = $address_field->address_number->value();
    $full_address = $address_field->address_line_1->value() . "\n";
    $full_address .= $address_field->address_line_2->value() . "\n";
    $full_address .= $address_field->address_line_3->value();
    $data = array(
      'ContactNumber' => $this->membershipNumber,
      'Address' => $full_address,
      'Town' => $address_field->town->value(),
      'County' => $address_field->county->value(),
      'Postcode' => $address_field->postcode->value(),
      'Country' => $address_field->country_code->value(),
      'Default' => ($default ? 'Y' : 'N'));
    $old_data = array(
      'ContactNumber' => $this->membershipNumber,
      'Default' => 'N',
      'ValidTo' => date('d/m/Y H:i:s'));
    if ($address_number) {
      $data['AddressNumber'] = $address_number;
      $old_data['AddressNumber'] = $address_number;
      if ($address_field->local_unsent_changes->value() == 1) {
        // Update existing address.
        if (variable_get('care_user_keep_history', TRUE)) {
          // Invalidate old address record, without changing it.
          $resultxml = care_call_method('UpdateAddress', $old_data);
          if (!isset($resultxml->ErrorMessage)) {
            // Create new address record and update AddressNumber.
            unset($data['AddressNumber']);
            $resultxml = care_call_method('AddAddress', $data);
            if (!isset($resultxml->ErrorMessage)) {
              $address_field->address_number->set((string) $resultxml->AddressNumber);
              $success = TRUE;
            }
          }
        }
        else {
          // Update existing address.
          $resultxml = care_call_method('UpdateAddress', $data);
          if (!isset($resultxml->ErrorMessage)) {
            $success = TRUE;
          }
        }
      }
      elseif ($address_field->local_unsent_changes->value() == 2) {
        // Delete existing address.
        $resultxml = care_call_method('UpdateAddress', $old_data);
        if (!isset($resultxml->ErrorMessage)) {
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
      $resultxml = care_call_method('AddAddress', $data);
      if (!isset($resultxml->ErrorMessage)) {
        $address_field->address_number->set((string) $resultxml->AddressNumber);
        $success = TRUE;
      }
    }
    if ($success and $address_field->value()) {
      // Record that we've saved changes to CARE.
      $address_field->local_unsent_changes->set(0);
    }
    return $success;
  }

  /**
   * Save single email address to CARE.
   */
  protected function saveEmail(EntityMetadataWrapper &$email_field, $default = FALSE) {
    $success = FALSE;
    $communication_number = $email_field->communication_number->value();
    $data = array(
      'ContactNumber' => $this->membershipNumber,
      'OldContactNumber' => $this->membershipNumber,
      'Number' => $email_field->email->value(),
      'Device' => $email_field->device_code->value(),
      'DeviceDefault' => ($default ? 'Y' : 'N'));
    $old_data = array(
      'OldContactNumber' => $this->membershipNumber,
      'DeviceDefault' => 'N',
      'ValidTo' => date('d/m/Y H:i:s'));
    if ($communication_number) {
      $data['CommunicationNumber'] = $communication_number;
      $old_data['CommunicationNumber'] = $communication_number;
      if ($email_field->local_unsent_changes->value() == 1) {
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
      elseif ($email_field->local_unsent_changes->value() == 2) {
        // Invalidate email record.
        $resultxml = care_call_method('UpdateCommunicationsNumber', $old_data);
        if (!isset($resultxml->ErrorMessage)) {
          $email_field->set(NULL);
          $success = TRUE;
        }
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
    if ($success and $email_field->value()) {
      // Record that we've saved changes to CARE.
      $email_field->local_unsent_changes->set(0);
    }
    return $success;
  }

  /**
   * Save single telephone number to CARE.
   */
  protected function saveTelephone(EntityMetadataWrapper &$telephone_field, $default = FALSE) {
    $success = FALSE;
    $communication_number = $telephone_field->communication_number->value();
    $data = array(
      'ContactNumber' => $this->membershipNumber,
      'OldContactNumber' => $this->membershipNumber,
      'Number' => $telephone_field->telephone->value(),
      'Device' => $telephone_field->device_code->value(),
      'DeviceDefault' => ($default ? 'Y' : 'N'));
    $old_data = array(
      'OldContactNumber' => $this->membershipNumber,
      'DeviceDefault' => 'N',
      'ValidTo' => date('d/m/Y H:i:s'));
    if ($communication_number) {
      $data['CommunicationNumber'] = $communication_number;
      $old_data['CommunicationNumber'] = $communication_number;
      if ($telephone_field->local_unsent_changes->value() == 1) {
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
      elseif ($telephone_field->local_unsent_changes->value() == 2) {
        // Delete existing address.
        $resultxml = care_call_method('UpdateCommunicationsNumber', $old_data);
        if (!isset($resultxml->ErrorMessage)) {
          $telephone_field->set(NULL);
          $success = TRUE;
        }
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
    if ($success and $telephone_field->value()) {
      // Record that we've saved changes to CARE.
      $telephone_field->local_unsent_changes->set(0);
    }
    return $success;
  }
}
