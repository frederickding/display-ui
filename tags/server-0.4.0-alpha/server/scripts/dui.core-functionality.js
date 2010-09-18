/**
 * Display UI core functionality
 *
 * Copyright 2009 Frederick Ding<br />
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain the full licensing terms for this project at
 * 		http://code.google.com/p/display-ui/wiki/License
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @version $Id$
 */
var DisplayUI = {
	/**
	 * Fades in the sidebar only if this is the first time the dashboard is loaded.
	 * PHP must add the 'first-load' class to the body tag if the user just logged in.
	 */
	showSidebar : function() {
		if ($j('body').hasClass('first-load')) {
			$j('div#sidebar').hide().animate( {
				opacity : 1.0
			}, 500).fadeIn(1000);
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
	toggleWidget : function(widget) {
		if (widget.hasClass('collapsed')) {
			widget.removeClass('no-after').removeClass('collapsed').next(
					'.widgets-widget-content').slideDown('fast');
		} else {
			widget.next('.widgets-widget-content').slideUp('fast', function() {
				widget.addClass('no-after').addClass('collapsed')
			});
		}
	}
}