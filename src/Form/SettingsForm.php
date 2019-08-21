<?php

namespace Drupal\care\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;
use \SoapClient;
use \Exception;

/**
 * Provides a test form object.
 */
class SettingsForm extends ConfigFormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId(): string {
    return 'care_settings_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state): array {

    $config = $this->config('care.settings');

    $form['care_wsdl_url'] = [
      '#title' => t('CARE WSDL URL'),
      '#type' => 'textfield',
      '#description' => t('Use the button below to test the URL without saving it.'),
      '#length' => 50,
      '#default_value' => $config->get('care_wsdl_url'),
    ];

    $form['test_wsdl'] = [
      '#value' => t('Test WSDL URL'),
      '#type' => 'submit',
      '#submit' => [
        '::testWsdl',
      ],
    ];

    $form['care_test wsdl_url'] = [
      '#title' => t('CARE test WSDL URL'),
      '#type' => 'textfield',
      '#description' => t('Use the button below to test the test URL without saving it.'),
      '#length' => 50,
      '#default_value' => $config->get('care_test_wsdl_url'),
    ];

    $form['test_test_wsdl'] = [
      '#value' => t('Test test WSDL URL'),
      '#type' => 'submit',
      '#submit' => [
        '::testTestWsdl',
      ],
    ];

    $form['care_doc_root'] = [
      '#title' => t('CARE documentation URL'),
      '#type' => 'textfield',
      '#description' => t('Home page for CARE API documentation.'),
      '#length' => 50,
      '#default_value' => $config->get('care_doc_root'),
    ];

    $form['logging'] = [
      '#title' => 'Logging Options',
      '#type' => 'fieldset',
    ];

    $form['logging']['care_log_calls'] = [
      '#title' => 'Log calls to CARE',
      '#type' => 'radios',
      '#options' => [
        1 => 'Yes',
        0 => 'No',
      ],
      '#default_value' => $config->get('care_log_calls'),
    ];

    $form['logging']['care_log_results'] = [
      '#title' => 'Log results from CARE',
      '#type' => 'radios',
      '#options' => [
        'full' => 'Full',
        'redacted' => 'Redacted',
        'none' => 'No results logging',
      ],
      '#default_value' => $config->get('care_log_results'),
    ];

    $form['actions'] = [
      '#type' => 'actions',
    ];
    $form['actions']['submit'] = [
      '#type' => 'submit',
      '#value' => $this->t('Save'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    if (trim($form_state->getValue('care_wsdl_url')) === '') {
      $form_state->setErrorByName('care_wsdl_url', $this->t('Please enter a WSDL URL for CARE.'));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    parent::submitForm($form, $form_state);
    $this->config('care.settings')
      ->set('care_wsdl_url', $form_state->getValue('care_wsdl_url'))
      ->save();
    $this->config('care.settings')
      ->set('care_test_wsdl_url', $form_state->getValue('care_test_wsdl_url'))
      ->save();
    $this->config('care.settings')
      ->set('care_doc_root', $form_state->getValue('care_doc_root'))
      ->save();
    $this->config('care.settings')
      ->set('care_log_calls', $form_state->getValue('care_log_calls'))
      ->save();
    $this->config('care.settings')
      ->set('care_log_results', $form_state->getValue('care_log_results'))
      ->save();
  }

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames(): array {
    return [
      'care.settings',
    ];
  }

  /**
   * Test that the supplied WSDL URL works for SoapClient.
   *
   * @noinspection PhpUnused
   *
   * @param array $form
   * @param FormStateInterface $form_state
   */
  public function testWsdl(array &$form, FormStateInterface $form_state) {
    $url = $form_state->getValue('care_wsdl_url');
    try {
      /** @noinspection PhpUnusedLocalVariableInspection */
      $client = @new SoapClient($url);
      $this->messenger()->addMessage(t('CARE WSDL URL %url is OK.', [
        '%url' => $url,
      ]));
      $this->submitForm($form, $form_state);
    }
    catch (Exception $e) {
      $this->messenger()->addError(t('CARE WSDL URL %url failed.', [
        '%url' => $url,
      ]));
      $this->messenger()->addError(t('Reverted to previous value.'));
    }
  }

  /**
   * Test that the supplied WSDL URL works for SoapClient.
   *
   * @noinspection PhpUnused
   *
   * @param array $form
   * @param FormStateInterface $form_state
   */
  public function testTestWsdl(array &$form, FormStateInterface $form_state) {
    $url = $form_state->getValue('care_test_wsdl_url');
    try {
      /** @noinspection PhpUnusedLocalVariableInspection */
      $client = @new SoapClient($url);
      $this->messenger()->addMessage(t('CARE test WSDL URL %url is OK.', [
        '%url' => $url,
      ]));
      $this->submitForm($form, $form_state);
    }
    catch (Exception $e) {
      $this->messenger()->addError(t('CARE test WSDL URL %url failed.', [
        '%url' => $url,
      ]));
      $this->messenger()->addError(t('Reverted to previous value.'));
    }
  }

}
