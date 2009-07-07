<?php
/**
 * IndexController for Admin module
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
 * The default controller for the Admin module
 */
class Admin_IndexController extends Zend_Controller_Action
{
	public function init()
	{
		$this->base_uri = explode('/admin', $_SERVER['REQUEST_URI']);
		$this->base_uri = $this->base_uri[0];
		// initiate a session for the installer
		$this->auth_session = new Zend_Session_Namespace('auth');
	}
	public function indexAction()
	{
		// TODO implement actions
		$this->_helper->viewRenderer->setNoRender();
		if(!$this->auth_session->authenticated) {
			$this->_redirect('http://'.$_SERVER['SERVER_NAME'].$this->base_uri.'/admin/login/');
		}
	}
}