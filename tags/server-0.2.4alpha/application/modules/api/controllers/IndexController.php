<?php
/**
 * IndexController for API
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
 * The default controller for the API module
 */
class Api_IndexController extends Zend_Controller_Action
{
	/**
	 * Default action
	 *
	 * Show the API error page
	 */
	public function indexAction ()
	{
		$this->getResponse()->setHttpResponseCode(400);
	}
	/**
	 * API key generator
	 *
	 * Generates an API key for the given system name
	 * In the future, should be REMOVED
	 * @todo Move this to protected admin area
	 */
	public function getkeyAction ()
	{
		$Authenticator = new Api_Model_Authenticator();
		$sys_name = $this->_getParam('sys_name');
		$this->getResponse()->setHeader('X-API-Key', $Authenticator->apiKey($sys_name));
		$this->_helper->viewRenderer->setNoRender();
	}
}