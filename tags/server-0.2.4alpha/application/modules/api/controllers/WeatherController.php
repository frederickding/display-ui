<?php
/**
 * Weather API controller
 *
 * Copyright 2009 Frederick Ding<br />
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 * 		http://www.apache.org/licenses/LICENSE-2.0
 * or the full licensing terms for this project at
 * 		http://code.google.com/p/display-ui/wiki/License
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Frederick
 * @license http://code.google.com/p/display-ui/wiki/License Apache License 2.0
 * @version $Id$
 */
/**
 * Provides plain-text API functions for retrieving weather data
 */
class Api_WeatherController extends Zend_Controller_Action
{
	/**
	 * Default method
	 *
	 * Forwards requests to unnamed actions to the current conditions action
	 */
	public function indexAction ()
	{
		$this->_forward('current');
	}
	/**
	 * Current conditions method
	 *
	 * Provides a plain-text API to retrieve the current conditions
	 */
	public function currentAction ()
	{
		// don't let Zend Framework render a view
		$this->_helper->viewRenderer->setNoRender();
		$YWeather = new Api_Model_YWeather($this->_getParam('location'));
		// authenticate the request
		$Authenticator = new Api_Model_Authenticator();
		$sys_name = $this->_getParam('sys_name');
		$signature = $this->_getParam('sig');
		if ($Authenticator->verify($sys_name, $signature)) {
			// target format: CC TT\nSL DESCRIPTION
			$this->getResponse()->setHeader('Content-Type', 'text/plain', true)
								->setBody($YWeather->conditions('code').' '
								.$YWeather->conditions('temp')."\n"
								.strlen($YWeather->conditions('description')).' '
								.strtolower($YWeather->conditions('description')));
		} else {
			$this->getResponse()->setHttpResponseCode(401);
			$this->view->message = 'Unauthenticated Request';
			$this->view->explanation = 'All API calls to this service '
				. 'must include a valid system identifier and request signature.';
			$this->render('index');
		}
	}
	/**
	 * 2-day forecast method
	 *
	 * Provides a plain-text API to retrieve the forecast for the next 2 days
	 */
	public function forecastAction ()
	{
		// don't let Zend Framework render a view
		$this->_helper->viewRenderer->setNoRender();
		$YWeather = new Api_Model_YWeather($this->_getParam('location'));
		// authenticate the request
		$Authenticator = new Api_Model_Authenticator();
		$sys_name = $this->_getParam('sys_name');
		$signature = $this->_getParam('sig');
		if ($Authenticator->verify($sys_name, $signature)) {
			$this->getResponse()->setHeader('Content-Type', 'text/plain', true);
			// target format: CC HI LO\nSL DESCRIPTION 
			switch ($this->_getParam('day')) {
				case 1:
					$this->getResponse()->setBody(
						$YWeather->forecast(1, 'code') . ' '
						.$YWeather->forecast(1, 'high') . ' '
						.$YWeather->forecast(1, 'low') . "\n"
						.strlen($YWeather->forecast(1, 'description')) . ' '
						.strtolower($YWeather->forecast(1, 'description')));
					break;
				case 0:
					$this->getResponse()->setBody(
						$YWeather->forecast(0, 'code') . ' '
						.$YWeather->forecast(0, 'high') . ' '
						.$YWeather->forecast(0, 'low') . "\n"
						.strlen($YWeather->forecast(0, 'description')) . ' '
						.strtolower($YWeather->forecast(0, 'description')));
					break;
				default:
					$this->getResponse()->setBody(
						$YWeather->forecast(0, 'code') . ' '
						.$YWeather->forecast(0, 'high') . ' '
						.$YWeather->forecast(0, 'low') . "\n"
						.strlen($YWeather->forecast(0, 'description')) . ' '
						.strtolower($YWeather->forecast(0, 'description')))
					->appendBody("\n"
						.$YWeather->forecast(1, 'code') . ' '
						.$YWeather->forecast(1, 'high') . ' '
						.$YWeather->forecast(1, 'low') . "\n"
						.strlen($YWeather->forecast(1, 'description')) . ' '
						.strtolower($YWeather->forecast(1, 'description')));
			}
		} else {
			$this->getResponse()->setHttpResponseCode(401);
			$this->view->message = 'Unauthenticated Request';
			$this->view->explanation = 'All API calls to this service '
				. 'must include a valid system identifier and request signature.';
			$this->render('index');
		}
	}
	/**
	 * Image retrieval method
	 *
	 * Redirects to the appropriate image for the specified condition,
	 * or, in the absence of a specified condition code, redirects to the
	 * image for the current weather condition
	 */
	public function imageAction ()
	{
		// don't let Zend Framework render a view
		$this->_helper->viewRenderer->setNoRender();
		$_code = ($this->_getParam('code')) ? $this->_getParam('code') : '34';
		if($this->getRequest()->getParam('location')) {
			$YWeather = new Api_Model_YWeather($this->getRequest()->getParam('location'));
			$_code = $YWeather->conditions('code');
			unset($YWeather);
		}
		$image = 'http://l.yimg.com/a/i/us/nws/weather/gr/'.$_code.'d.png';
		$this->_redirect($image);
	}
}