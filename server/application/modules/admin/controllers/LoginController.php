<?php
/**
 * Login for Admin module
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
 * The login controller for the Admin module
 */
class Admin_LoginController extends Zend_Controller_Action
{
	public function init()
	{
		$this->base_uri = explode('/admin/login', $_SERVER['REQUEST_URI']);
		$this->base_uri = $this->base_uri[0];
		// initiate a session for the installer
		$this->session = new Zend_Session_Namespace('auth');
	}
	public function indexAction()
	{
		$this->view->base_uri = $this->base_uri;
		$_user = addslashes($this->_getParam('username'));
	}
	public function submitAction()
	{
		$this->_helper->viewRenderer->setNoRender();
		$_user = addslashes($this->_getParam('username'));
		$_password = trim($this->_getParam('password'));
		$Auth = new Admin_Model_Authentication();
		if($Auth->checkPassword($_user, $_password)) echo TRUE;
		else {
			$this->view->valid = FALSE;
			$this->view->username = $_user;
			$this->_redirect('http://'.$_SERVER['SERVER_NAME'].$this->base_uri.'/admin/login/?username='.$_user);
		}
	}
}