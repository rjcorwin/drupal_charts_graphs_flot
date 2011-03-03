/**
* @description SerializeTable
* @author Heberti Almeida
* @email hebertialmeida@gmail.com
* @github https://github.com/hebertialmeida/Serialize-Table
* @version 1.0 --> 2010-11-25
* @license GNU LGPL (http://www.gnu.org/licenses/lgpl.html)
*/
(function( $ ){
	$.fn.serializeTable = function(opt) {
		var defaults = {
			file: "",
			params: "params",
			data: "#content",
			attr: "rel",
			loading_text: "",
			loading_class: "serializetable-ldg"
		};
		opt = $.extend(defaults, opt);					
		$("tr", this).live("click", function(){	
			$(this).addClass("clicked");
			var str = "";						
			var arr = "";
			$("td", this).each(function(id) {
				name = $(".clicked td").eq(id).attr(opt.attr);
				str += '"'+name+'":"'+($(this).text()+'').replace(/"/g, '%26quot%3B').replace(/&/g, '%26').replace(/#/g, '%23')+'",';
			});
			$(this).removeClass("clicked");
			if(str) {		
				str = "[{"+str.replace(/(\s+)?.$/, "")+"}]";
				
				$.ajax({
					url: opt.file+'?'+opt.params+'='+str,
					beforeSend:  function() {
						if (opt.loading_text != "") $(opt.data).html('<span class="'+opt.loading_class+'">'+opt.loading_text+'</span>');
					},
					success: function(data) {
						$(opt.data).find(opt.loading_class).remove();
						$(opt.data).html(data);
					}
				});
			}
		});
	};
})( jQuery );