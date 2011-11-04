<?php

/**
 * @file pinapi-redeem-form.tpl.php
 * Default theme implementation for displaying a redeem form.
 *
 * Available variables:
 * - $pinapi_redeem_form: The complete redeem form ready for print.
 * - $pinapi_redeem: Array of keyed form elements. Can be used to print each form element separately.
 * - $group: A group object.
 *
 * @see template_preprocess_pinapi_redeem_form()
 */
?>
<div class="pinapi-redeem-<?php print $group->gid; ?>">
  <?php print $pinapi_redeem_form; ?>
</div>
