(function($){

Drupal.behaviors.pinapi_tester = {
  attach: function(context, settings) {
    $.each(Drupal.settings.pinapi_tester, function(selector, options){
      $('ul.pinapi-tester-codes li', selector).css('cursor', 'pointer').click(function(){
        $('#edit-code', options.form).val( $(this).data('code') );
      });
    });
  }
};

})(jQuery);
