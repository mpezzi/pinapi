<?php

/**
 * A handler to provide proper displays for booleans.
 *
 * Allows for display of true/false, yes/no, on/off.
 *
 * Definition terms:
 *   - output formats: An array where the first entry is displayed on boolean true
 *      and the second is displayed on boolean false. An example for sticky is:
 *      @code
 *      'output formats' => array(
 *        'sticky' => array(t('Sticky'), ''),
 *      ),
 *      @endcode
 *
 * @ingroup views_field_handlers
 */
class pinapi_paypal_views_handler_field_payment_status extends views_handler_field {
  function option_definition() {
    $options = parent::option_definition();
    $options['type'] = array('default' => 'string');

    return $options;
  }

  function init(&$view, $options) {
    parent::init($view, $options);

    $default_formats = array(
      'string' => array(t('Incomplete'), t('Complete'), t('Pending'), t('Failed')),
    );
    $output_formats = isset($this->definition['output formats']) ? $this->definition['output formats'] : array();
    $this->formats = array_merge($default_formats, $output_formats);
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    foreach ($this->formats as $key => $item) {
      $options[$key] = implode('/', $item);
    }

    $form['type'] = array(
      '#type' => 'select',
      '#title' => t('Output format'),
      '#options' => $options,
      '#default_value' => $this->options['type'],
    );
  }

  function render($values) {
    $value = $values->{$this->field_alias};
    return isset($this->formats[$this->options['type']][$value]) ? $this->formats[$this->options['type']][$value] : t('Incomplete');
  }
}
