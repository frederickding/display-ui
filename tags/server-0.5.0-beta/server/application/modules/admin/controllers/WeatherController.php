<?php
/**
 * Weather for Admin module
 *
 * Copyright 2010 Frederick Ding<br />
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * or the full licensing terms for this project at
 * http://code.google.com/p/display-ui/wiki/License
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
require 'ControllerAbstract.php';
/**
 * Functionality for weather
 */
class Admin_WeatherController extends Admin_ControllerAbstract
{
	public function indexAction ()
	{
		$this->_forward('list');
	}
	public function listAction ()
	{
		$WeatherModel = new Admin_Model_Weather();
		$list = $WeatherModel->fetchClients($this->auth_session->username);
		$this->view->clientsList = $list;
	}
}