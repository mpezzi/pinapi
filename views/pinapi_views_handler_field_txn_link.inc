<?php

/**
 * Field handler to present a link to the user.
 */

class pinapi_views_handler_field_txn_link extends views_handler_field {

  function construct() {
    parent::construct();
    $this->additional_fields['txn_id'] = 'txn_id';
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['text'] = array('default' => '', 'translatable' => TRUE);
    return $options;
  }

  function option_form(&$form, &$form_state) {
    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#default_value' => $this->options['text'],
    );
    parent::options_form($form, $form_state);
  }

  function access() {
    return user_access('administer pins');
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $value = $this->get_value($values, 'txn_id');
    return $this->render_link($this->sanitize_value($value), $values);
  }

  function render_link($data, $values) {
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');

    $this->options['alter']['make_link'] = TRUE;
    $this->options['alter']['path'] = "admin/structure/pin/transaction/" . $data;

    return $text;
  }

}
