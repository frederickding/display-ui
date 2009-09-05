var DisplayUI = {
	/**
	 * Fades in the sidebar only if this is the first time the dashboard is loaded.
	 * PHP must add the 'first-load' class to the body tag if the user just logged in.
	 */
	showSidebar : function () {
		if($j('body').hasClass('first-load')) {
			$j('div#sidebar').hide().animate({opacity: 1.0}, 1000).fadeIn(2000);
		}
	},
	/**
	 * Animates the logout process with cool effects
	 */
	logout : function () {
		$j.blockUI({ message: '<h2>Logging out&hellip;</h2>' });
		$j('#yui-main div.yui-b').fadeOut(1000,
			function() { $j('div#hd').fadeOut(1000,
				function() { $j('div#sidebar').fadeOut(1000,
					function() { document.location = $j('li#sidebar-nav-logout a').attr('href') })
			})
		});
	},
	/**
	 * Toggles the main widget body when the title of the widget is clicked.
	 * Expects a jQuery object as the parameter
	 */
	toggleWidget : function (widget) {
		if(widget.hasClass('collapsed')) {
			widget.removeClass('no-after').removeClass('collapsed').next('.widgets-widget-content').slideDown('fast');
		} else {
			widget.next('.widgets-widget-content').slideUp('fast', function() {widget.addClass('no-after').addClass('collapsed')});
		}
	}
}