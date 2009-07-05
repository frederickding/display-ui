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
class Admin_IndexController extends Zend_Controller_Action{
	public function indexAction()
	{
		// TODO implement actions
	}
	public function loginAction()
	{
		// TODO implement login
		$_user = $this->getRequest()->getParam('username');
		$_password = trim($this->getRequest()->getParam('password'));
		$Auth = new Admin_Model_Authentication();
		if($Auth->checkPassword($_user, $_password))
			$this->_helper->viewRenderer()->setNoRender();
		else {
			$this->view->valid = FALSE;
			$this->view->username = $_user;
		}
	}
}