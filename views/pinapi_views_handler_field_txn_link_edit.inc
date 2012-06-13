<?php

/**
 * Field handler to present a link to edit transaction.
 */
class pinapi_views_handler_field_txn_link_edit extends pinapi_views_handler_field_txn_link {

  function render_link($data, $values) {
    if ( $data ) {
      $this->options['alter']['make_link'] = TRUE;

      $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');

      $this->options['alter']['path'] = "pin/transaction/$data/edit";
      $this->options['alter']['query'] = drupal_get_destination();

      return $text;
    }
  }

}