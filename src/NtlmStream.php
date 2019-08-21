<?php

interface NtlmStreamWrapperInterface {

  public function stream_open(string $path, string $mode, int $options, string &$opened_path);

  public function stream_read(int $count);

  public function stream_stat();

  public function url_stat(string $path, int $flags);

  public function stream_eof();

  public function stream_tell();

  public function stream_flush();

  public function stream_close();
}

class NtlmStream implements NtlmStreamWrapperInterface {

  private $ch;

  private $path;

  private $mode;

  private $options;

  private $opened_path;

  private $buffer;

  private $pos;

  private static $user;

  private static $password;

  public static function setUserPass($username, $password) {
    self::$user = $username;
    self::$password = $password;
  }

  /**
   * Open the stream
   *
   * @param string $path
   *   The URL to open.
   * @param string $mode
   *   Mode to use for as for fopen().
   * @param int $options
   *
   * @param string $opened_path
   *
   * @return bool
   */
  public function stream_open(string $path, string $mode, int $options, string &$opened_path): bool {
    $this->path = $path;
    $this->mode = $mode;
    $this->options = $options;
    $this->opened_path = $opened_path;
    $this->createBuffer($path);
    return TRUE;
  }

  /**
   * Close the stream
   *
   */
  public function stream_close() {
    curl_close($this->ch);
  }

  /**
   * Read the stream
   *
   * @param int $count
   *   Number of bytes to read
   *
   * @return bool|string
   *   Content from pos to count.
   *
   * @noinspection PhpUnused
   */
  public function stream_read(int $count) {
    if ($this->buffer === '') {
      return FALSE;
    }
    $read = substr($this->buffer, $this->pos, $count);
    $this->pos += $count;
    return $read;
  }

  /**
   * write the stream
   *
   * @param string $data
   *   Data to write to the stream.
   *
   * @return bool
   *   Whether we have anything to write.
   */
  /*public function stream_write($data) {
    return !($this->buffer === '');
  }*/

  /**
   *
   * @return bool
   *    True if eof else False
   */
  public function stream_eof(): bool {
    return $this->pos > strlen($this->buffer);
  }

  /**
   * @return int
   *   The position of the current read pointer
   */
  public function stream_tell(): int {
    return $this->pos;
  }

  /**
   * Flush stream data.
   */
  public function stream_flush() {
    $this->buffer = NULL;
    $this->pos = NULL;
  }

  /**
   * Stat the file, return only the size of the buffer
   *
   * @return array
   *   Stat information
   */
  public function stream_stat(): array {
    $this->createBuffer($this->path);
    return [
      'size' => strlen($this->buffer),
    ];
  }

  /**
   * Stat the url, return only the size of the buffer
   *
   * @param string $path
   * @param int $flags
   *
   * @return array
   *   Stat information
   */
  public function url_stat(string $path, int $flags): array {
    $this->createBuffer($path);
    return [
      'size' => strlen($this->buffer),
    ];
  }

  /**
   * Create the buffer by requesting the url through cURL
   *
   * @param string $path
   */
  private function createBuffer($path) {
    if ($this->buffer) {
      return;
    }
    $this->ch = curl_init($path);
    curl_setopt($this->ch, CURLOPT_RETURNTRANSFER, TRUE);
    curl_setopt($this->ch, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_setopt($this->ch, CURLOPT_HTTPAUTH, CURLAUTH_NTLM);
    curl_setopt($this->ch, CURLOPT_USERPWD, self::$user . ':' . self::$password);
    $this->buffer = curl_exec($this->ch);
    $this->pos = 0;
  }
}
