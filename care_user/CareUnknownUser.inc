<?php

/**
 * @file
 * Define an object to match with contacts in CARE.
 */

/**
 * Class CareUnknownUser Finds matches in CARE.
 */
class CareUnknownUser extends CareUser {

  protected $testEmail;

  protected $testAddressField;

  protected $female_titles = [
    'Ms',
    'Miss',
    'Mrs',
  ];

  protected $unisex_titles = [
    'Dr',
    'Prof',
    'Rev',
  ];

  /**
   * Constructor.
   */
  public function __construct($account = NULL) {
    if (!$account) {
      $account = user_load(0, TRUE);
    }
    parent::__construct($account);
  }

  /**
   * Set this user's membership number, if known.
   */
  public function setMembershipNumber($contact_number) {
    $this->membershipNumber = $contact_number;
  }

  /**
   * Set the name field to one from a form.
   */
  public function setNameField($care_name_field) {
    if ($care_name_field && $care_name_field->value()) {
      if ($this->nameField != $care_name_field) {
        $this->nameField = $care_name_field;
        $this->nameField->local_unsent_changes = CARE_FIELDS_CHANGED;
      }
    }
  }

  /**
   * Set the DoB field to one from a form.
   */
  public function setDobField($care_dob_field) {
    if ($care_dob_field && $care_dob_field->value()) {
      if ($this->dobField != $care_dob_field) {
        $this->dobField = $care_dob_field;
        $this->dobField->local_unsent_changes = CARE_FIELDS_CHANGED;
      }
    }
  }

  /**
   * Set the email address to find contacts for.
   */
  public function setTestEmail($email) {
    $this->testEmail = $email;
  }

  /**
   * Set the address field to one from a form.
   */
  public function setTestAddressField($address_field) {
    $this->testAddressField = $address_field;
  }

  /**
   * Find existing CARE contacts that might match this user.
   */
  public function findMatchingContacts() {
    watchdog('care_user', 'Finding possible matching CARE contacts');
    $matches = [];
    if ($this->testEmail) {
      // Find contacts with this account's email address.
      // ToDo: look for all email addresses for this account?
      $data = [
        'EMailAddress' => $this->testEmail,
      ];
      $resultxml = care_call_method('FindContacts', $data);
      foreach ($resultxml as $result) {
        $contact_number = (int) $result->ContactNumber;
        $data = [
          'ContactNumber' => $contact_number,
          'Email' => $this->testEmail,
          'Title' => (string) $result->Title,
          'Initial' => substr(trim((string) $result->Initials), 0, 1),
          'Forenames' => (string) $result->Forenames,
          'PreferredForename' => (string) $result->PreferredForename,
          'Surname' => (string) $result->Surname,
          'Address' => (string) $result->Address,
          'Town' => (string) $result->Town,
          'Postcode' => (string) $result->Postcode,
          'Matches' => [
            'Email',
          ],
          'Matchesfuzzy' => [],
          'Mismatches' => [],
          // 'Data' => _care_pretty_xml($result)
        ];
        $matches[$contact_number] = $data;
      }
    }
    // Find additional contacts with this account's surname.
    if ($this->nameField->value()) {
      // Look for contacts with this surname:
      if ($this->nameField->type() == 'care_name') {
        $surname = $this->nameField->surname->value();
      }
      else {
        $surname = $this->nameField->value();
      }
      $data = [
        'Surname' => $surname,
        'UseSearchNames' => 'Y',
        'UseSoundex' => 'Y',
      ];
      if ($this->testAddressField && $this->testAddressField->value() && $this->testAddressField->postcode->value()) {
        $data['Postcode'] = $this->testAddressField->postcode->value();
      }
      $resultxml = care_call_method('FindContacts', $data);
      foreach ($resultxml as $result) {
        $result_postcode = (string) $result->Postcode;
        $result_postcode = preg_replace('/\s/', '', $result_postcode);
        if (in_array($result_postcode, [
          '',
          'UNKNOWN',
        ])) {
          continue;
        }
        $contact_number = (int) $result->ContactNumber;
        $data = [
          'ContactNumber' => $contact_number,
          'Email' => '?',
          'Title' => (string) $result->Title,
          'Initial' => substr(trim((string) $result->Initials), 0, 1),
          'Forenames' => (string) $result->Forenames,
          'PreferredForename' => (string) $result->PreferredForename,
          'Surname' => (string) $result->Surname,
          'Address' => (string) $result->Address,
          'Town' => (string) $result->Town,
          'Postcode' => (string) $result->Postcode,
          'Matches' => [],
          'Matchesfuzzy' => [],
          'Mismatches' => [],
          // 'Data' => _care_pretty_xml($result)
        ];
        // Add any new ContactNumbers to the list.
        if (!isset($matches[$contact_number])) {
          $matches[$contact_number] = $data;
        }
      }
    }
    $candidate_count = count($matches);
    // Find matches and non-matches for all fields.
    foreach ($matches as $index => $data) {
      if (!in_array('Email', $data['Matches'])) {
        $matches[$index]['Mismatches'][] = 'Email';
      }
      if ($this->nameField && $this->nameField->value()) {
        // Match title, with some flexibility.
        if ($data['Title'] && isset($this->nameField->title) && $this->nameField->title->value()) {
          $our_title = $this->nameField->title->value();
          if ($data['Title'] == $our_title || trim($our_title) == '') {
            $matches[$index]['Matches'][] = 'Title';
          }
          elseif (in_array($data['Title'], $this->female_titles) && in_array($our_title, $this->female_titles)) {
            $matches[$index]['Matchesfuzzy'][] = 'Title';
          }
          elseif (in_array($data['Title'], $this->unisex_titles) or in_array($our_title, $this->unisex_titles)) {
            $matches[$index]['Matchesfuzzy'][] = 'Title';
          }
          else {
            $matches[$index]['Mismatches'][] = 'Title';
          }
        }
        // Match case-insensitively, or initial if CARE doesn't have forename.
        if (isset($this->nameField->forenames) && $this->nameField->forenames->value()) {
          $entered_forename = strtolower($this->nameField->forenames->value());
          if ($data['Forenames']) {
            if (strtolower($data['Forenames']) == $entered_forename or strtolower($data['PreferredForename']) == $entered_forename) {
              $matches[$index]['Matches'][] = 'Forenames';
            }
            else {
              $matches[$index]['Mismatches'][] = 'Forenames';
            }
          }
          elseif ($data['Initial']) {
            if ($data['Initial'] == substr($entered_forename, 0, 1)) {
              $matches[$index]['Matches'][] = 'Initial';
            }
            else {
              $matches[$index]['Mismatches'][] = 'Initial';
            }
          }
        }
        // Match surname case-insensitively.
        if ($data['Surname'] && $surname) {
          if (strtolower($data['Surname']) == strtolower($surname)) {
            $matches[$index]['Matches'][] = 'Surname';
          }
          else {
            $matches[$index]['Mismatches'][] = 'Surname';
          }
        }
      }
      if ($this->testAddressField && $this->testAddressField->value()) {
        // ToDo: look for all addresses for this account?
        $address1 = strtolower($this->testAddressField->address_line_1->value());
        $address2 = strtolower($this->testAddressField->address_line_2->value());
        $address3 = strtolower($this->testAddressField->address_line_3->value());
        $address_matches = [
          $address1,
          $address1 . ' ' . $address2,
          $address1 . ' ' . $address2 . ' ' . $address3,
          $address1 . ', ' . $address2,
          $address1 . ', ' . $address2 . ', ' . $address3,
          $address1 . "\n" . $address2,
          $address1 . "\n" . $address2 . "\n" . $address3,
        ];
        if (strtolower($data['Address']) && $address1) {
          if (in_array(strtolower($data['Address']), $address_matches)) {
            $matches[$index]['Matches'][] = 'Address';
          }
          else {
            $matches[$index]['Mismatches'][] = 'Address';
          }
        }
        if ($data['Town'] && $this->testAddressField->town->value()) {
          if (strtolower($data['Town']) == strtolower($this->testAddressField->town->value())) {
            $matches[$index]['Matches'][] = 'Town';
          }
          else {
            $matches[$index]['Mismatches'][] = 'Town';
          }
        }
        if ($data['Postcode'] && $this->testAddressField->postcode->value()) {
          if (strtolower(str_replace(' ', '', $data['Postcode'])) == strtolower(str_replace(' ', '', $this->testAddressField->postcode->value()))) {
            $matches[$index]['Matches'][] = 'Postcode';
          }
          else {
            $matches[$index]['Mismatches'][] = 'Postcode';
          }
        }
      }
    }
    // Sort by likelihood.
    uasort($matches, function ($a, $b) {
      $score_a = count($a['Matches']);
      $score_b = count($b['Matches']);
      if ($score_a == $score_b) {
        $mis_score_a = count($a['Mismatches']);
        $mis_score_b = count($b['Mismatches']);
        if ($mis_score_a == $mis_score_b) {
          return ($a['ContactNumber'] > $b['ContactNumber']);
        }
        return ($mis_score_a > $mis_score_b);
      }
      return ($score_a < $score_b);
    });
    // Remove definite mismatches.
    foreach ($matches as $index => $match) {
      $matches_email = in_array('Email', $match['Matches']);
      if ($this->nameField && $this->nameField->value()) {
        $matches_title = (in_array('Title', $match['Matches']) or in_array('Title', $match['Matchesfuzzy']));
        $matches_name = (in_array('Initial', $match['Matches']) or in_array('Forenames', $match['Matches']));
        $matches_surname = in_array('Surname', $match['Matches']);
        if (!$matches_title || !$matches_name || !$matches_surname) {
          unset($matches[$index]);
        }
      }
      if ($this->addressField && $this->addressField->value()) {
        $matches_postcode = in_array('Postcode', $match['Matches']);
        if (!$matches_email || !$matches_postcode) {
          unset($matches[$index]);
        }
      }
    }
    watchdog('care_user', 'Match summary: !candidate_count candidates, !match_count matches: Best match: !best_contact', [
      '!candidate_count' => $candidate_count,
      '!match_count' => count($matches),
      '!best_contact' => reset($matches)['ContactNumber'],
    ]);
    return $matches;
  }

  /**
   * Return the ContactNumber for the best match in CARE.
   */
  public function getBestMatchingContact() {
    if ($this->membershipNumber) {
      return $this->membershipNumber;
    }
    $matches = $this->findMatchingContacts();
    watchdog('care_user', 'Matching CARE contacts: <pre>!matches</pre>', [
      '!matches' => print_r($matches, TRUE),
    ]);
    if (count($matches)) {
      return reset($matches)['ContactNumber'];
    }
    return 0;
  }

}
