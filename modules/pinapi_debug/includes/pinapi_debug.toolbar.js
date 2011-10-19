$(function(){
  $('#pinapi-debug-toolbar h4').bind('click', function(){
    $(this).parents('div.pinapi-debug-pool').find('div.content').slideToggle(125);
  });
  
  $('#pinapi-debug-toolbar span.pin').click(function(){
    $('#edit-code').val( $(this).attr('data-code') );
  });
  
});