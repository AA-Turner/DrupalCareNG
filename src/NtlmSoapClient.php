<?php

/**
 * Soap Client using Microsoft's NTLM Authentication.
 *
 * Original source: https://github.com/jamesiarmes/php-ntlm
 */
class NtlmSoapClient extends SoapClient {

  /**
   * cURL resource used to make the SOAP request
   *
   * @var resource
   */
  protected $ch;

  protected $__last_response;

  /**
   * Options passed to the client constructor.
   *
   * @var array
   */
  private $options;

  /**
   * @var string
   */
  private $__last_request;

  /**
   * @var array
   */
  private $__last_request_headers;

  /**
   * @var bool
   */
  private $__last_response_headers;

  /**
   * {@inheritdoc}
   *
   * Additional options:
   * - user (string): The user to authenticate with.
   * - password (string): The password to use when authenticating the user.
   * - curlopts (array): Array of options to set on the curl handler when
   *   making the request.
   */
  public function __construct($wsdl, array $options = NULL) {
    // Set missing indexes to their default value.
    $options += [
      'user' => NULL,
      'password' => NULL,
      'curlopts' => [],
      'testwsdl' => FALSE,
    ];
    $this->options = $options;
    // Verify that a user name and password were entered.
    if (empty($options['user']) || empty($options['password'])) {
      throw new BadMethodCallException(
        'A username and password is required.'
      );
    }
    if ($options['testwsdl']) {
      // Test we can GET the WSDL.
      $this->ch = curl_init($wsdl);
      $options = $this->options['curlopts'] + [
          CURLOPT_HEADER => TRUE,
          CURLOPT_RETURNTRANSFER => TRUE,
          CURLOPT_HTTPAUTH => CURLAUTH_NTLM,
          CURLOPT_USERPWD => $this->options['user'] . ':'
            . $this->options['password'],
        ];
      curl_setopt_array($this->ch, $options);
      curl_exec($this->ch);
      $info = curl_getinfo($this->ch);
      if ($info['http_code'] === 302) {
        throw new RuntimeException(
          t('HTTP 302: WSDL URL was redirected to %url.', ['%url' => $info['redirect_url']]));
      }
      if ($info['http_code'] === 401) {
        throw new RuntimeException(
          t('HTTP 401: Unauthorized: Access is denied due to invalid NTLM credentials.'));
      }
    }
    else {
      // Do normal SoapClient construction, but using NTLM stream.
      NtlmStream::setUserPass($options['user'], $options['password']);
      stream_wrapper_unregister('http');
      stream_wrapper_unregister('https');
      stream_wrapper_register('https', 'NtlmStream');
      stream_wrapper_register('http', 'NtlmStream');
      parent::__construct($wsdl, $options);
      stream_wrapper_restore('http');
      stream_wrapper_restore('https');
    }
  }

  /**
   * {@inheritdoc}
   */
  public function __doRequest($request, $location, $action, $version, $one_way = 0) {
    $headers = $this->buildHeaders($action);
    $this->__last_request = $request;
    $this->__last_request_headers = $headers;
    // Only reinitialize curl handle if the location is different.
    if (!$this->ch
      || curl_getinfo($this->ch, CURLINFO_EFFECTIVE_URL) !== $location) {
      $this->ch = curl_init($location);
    }
    curl_setopt_array($this->ch, $this->curlOptions($action, $request));
    $response = curl_exec($this->ch);
    // If the response if false than there was an error and we should throw
    // an exception.
    if ($response === FALSE) {
      $this->__last_response = $this->__last_response_headers = FALSE;
      throw new RuntimeException(
        'Curl error: ' . curl_error($this->ch),
        curl_errno($this->ch)
      );
    }
    $info = curl_getinfo($this->ch);
    if ($info['http_code'] === 401) {
      throw new RuntimeException(
        'Unauthorized: Access is denied due to invalid credentials.'
      );
    }
    $this->parseResponse($response);
    return $this->__last_response;
  }

  /**
   * {@inheritdoc}
   */
  public function __getLastRequestHeaders() {
    return implode("\n", $this->__last_request_headers) . "\n";
  }

  /**
   * Builds the headers for the request.
   *
   * @param string $action
   *   The SOAP action to be performed.
   *
   * @return array
   */
  protected function buildHeaders($action) {
    return [
      'Method: POST',
      'Connection: Keep-Alive',
      'User-Agent: PHP-SOAP-CURL',
      'Content-Type: text/xml; charset=utf-8',
      "SOAPAction: \"$action\"",
      'Expect: 100-continue',
    ];
  }

  /**
   * Builds an array of curl options for the request
   *
   * @param string $action
   *   The SOAP action to be performed.
   * @param string $request
   *   The XML SOAP request.
   *
   * @return array
   *   Array of curl options.
   */
  protected function curlOptions($action, $request) {
    $options = $this->options['curlopts'] + [
        CURLOPT_RETURNTRANSFER => TRUE,
        CURLOPT_HTTPHEADER => $this->buildHeaders($action),
        CURLOPT_HTTPAUTH => CURLAUTH_NTLM,
        CURLOPT_USERPWD => $this->options['user'] . ':'
          . $this->options['password'],
      ];
    // We shouldn't allow these options to be overridden.
    $options[CURLOPT_HEADER] = TRUE;
    $options[CURLOPT_POST] = TRUE;
    $options[CURLOPT_POSTFIELDS] = $request;
    return $options;
  }

  /**
   * Parses the response from a successful request.
   *
   * @param string $response
   *   The response from the cURL request, including headers and body.
   */
  public function parseResponse($response) {
    // Parse the response and set the last response and headers.
    $info = curl_getinfo($this->ch);
    $this->__last_response_headers = substr(
      $response,
      0,
      $info['header_size']
    );
    $this->__last_response = substr($response, $info['header_size']);
  }
}
