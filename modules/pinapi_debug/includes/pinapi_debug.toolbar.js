$(function(){
  $('#pinapi-debug-toolbar h4').bind('click', function(){
    $(this).parents('div.pinapi-debug-group').find('div.content').slideToggle(125);
  });
  $('#pinapi-debug-toolbar span.pin').click(function(){
    $('form input.pinapi-code').val( $(this).attr('data-code') );
  });
});
