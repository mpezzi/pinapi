<?php
// $Id: views_handler_field_node.inc,v 1.7.2.2 2010/03/16 23:09:05 merlinofchaos Exp $
/**
 * @file
 * Contains the basic 'node' field handler.
 */

/**
 * Field handler to provide simple renderer that allows linking to a node.
 */
class pinapi_prize_views_handler_field_prize_redeemed extends views_handler_field {
  /**
   * Constructor to provide additional field to add.
   */
  function construct() {
    parent::construct();
    $this->additional_fields['rid'] = array('table' => 'pinapi_code_redeemed', 'field' => 'rid');
    /*
    if (module_exists('translation')) {
      $this->additional_fields['language'] = array('table' => 'node', 'field' => 'language');
    }
    */
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['link_to_prize_redeemed'] = array('default' => FALSE);
    return $options;
  }

  /**
   * Provide link to node option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['link_to_prize_redeemed'] = array(
      '#title' => t('Link this field to its redeemed prize'),
      '#description' => t('This will override any other link you have set.'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['link_to_prize_redeemed']),
    );
  }

  /**
   * Render whatever the data is as a link to the node.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($data, $values) {
    if (!empty($this->options['link_to_prize_redeemed']) && $data !== NULL && $data !== '') {
      $this->options['alter']['make_link'] = TRUE;
      $this->options['alter']['path'] = "prize/" . $values->{$this->aliases['rid']};
      /*
      if (isset($this->aliases['language'])) {
        $languages = language_list();
        if (isset($languages[$values->{$this->aliases['language']}])) {
          $this->options['alter']['language'] = $languages[$values->{$this->aliases['language']}];
        }
        else {
          unset($this->options['alter']['language']);
        }
      }
      */
    }
    return $data;
  }

  function render($values) {
    return $this->render_link(check_plain($values->{$this->field_alias}), $values);
  }
}
