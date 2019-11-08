<?php

/**
 * Implements hook_views_data().
 *
 * @noinspection PhpUnused
 */
function care_taxonomy_views_data() {
    $data['taxonomy_vocabulary']['machine_name_string'] = [
    'title' => t('Machine name as string'),
    'description' => t('The vocabulary machine name, treated as a string.'),
    'real field' => 'machine_name',
    'filter' => [
      'handler' => 'views_handler_filter_string',
    ],
  ];
  return $data;
}
