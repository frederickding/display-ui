<?php
/**
 * Login for Admin module
 *
 * Copyright 2009-2010 Frederick Ding<br />
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
/**
 * The login controller for the Admin module
 */
class Admin_LoginController extends Zend_Controller_Action
{
	public function init ()
	{
		// initiate a session for the installer
		$this->session = new Zend_Session_Namespace('auth');
		// make the ACL
		if (! Zend_Registry::isRegistered('dui_acl')) {
			$acl = new Admin_Model_Acl();
			Zend_Registry::set('dui_acl', $acl);
		}
	}
	public function indexAction ()
	{
		$_redir = $this->_getParam('redirect', '');
		if ($this->session->authenticated) {
			// no need to log in again, send them on their way
			if (! empty($_redir) && $_redir[0] == '/')
				$this->_redirect($this->view->serverUrl() . $_redir);
			else
				$this->_redirect(
				$this->view->serverUrl() . $this->view->url(
				array(
					'module' => 'admin',
					'controller' => 'index',
					'action' => 'dashboard')));
		}
		$this->view->username = preg_replace('/[^a-zA-Z0-9\s]/', '',
		$this->_getParam('username'));
		if (! empty($_redir) && $_redir[0] == '/')
			$this->view->assign('redirect', $_redir);
		else
			$this->view->assign('redirect', '');
		if ($this->_getParam('failed', false) &&
		 isset($this->session->loginErrorDetails)) {
			$this->view->assign('loginErrorDetails',
			$this->session->loginErrorDetails);
		} else {
			unset($this->view->loginErrorDetails);
		}
	}
	public function submitAction ()
	{
		$this->_helper->viewRenderer->setNoRender();
		$_user = preg_replace('/[^a-zA-Z0-9\s]/', '',
		$this->_getParam('username'));
		$_password = $this->_getParam('password');
		$_redir = $this->_getParam('redirect');
		$_yubikey = $this->_getParam('yubikey');
		$Auth = new Admin_Model_Authentication();
		$passwordValid = $Auth->checkPassword($_user, $_password);
		$yubikeyValid = false;
		if ($passwordValid)
			$yubikeyValid = $Auth->checkYubikey($_user, $_yubikey);
		if ($passwordValid && $yubikeyValid) {
			$this->session->authenticated = true;
			$this->session->username = $_user;
			unset($this->session->loginDebug);
			if (! empty($_redir) && $_redir[0] == '/') {
				$this->_redirect($this->view->serverUrl() . $_redir);
			} else {
				// send them to the dashboard
				$this->_redirect(
				$this->view->serverUrl() . $this->view->url(
				array(
					'module' => 'admin',
					'controller' => 'index',
					'action' => 'dashboard')));
			}
		} else {
			$this->session->loginErrorDetails = (! $passwordValid) ? 'userpass' : ((! $yubikeyValid) ? 'yubikey' : 'unknown');
			$this->session->authenticated = false;
			if (! empty($_redir) && $_redir[0] == '/') {
				$this->_redirect(
				$this->view->serverUrl() .
				 $this->view->url(
				array(
					'module' => 'admin',
					'controller' => 'login',
					'action' => 'index')) . '?failed=1&username=' . $_user .
				 '&redirect=' . $_redir);
			} else {
				$this->_redirect(
				$this->view->serverUrl() . $this->view->url(
				array(
					'module' => 'admin',
					'controller' => 'login',
					'action' => 'index')) . '?failed=1&username=' . $_user);
			}
		}
	}
	public function logoutAction ()
	{
		$this->_helper->layout()->setLayout('SimpleBlue');
		$this->session->authenticated = false;
		unset($this->session->username);
	}
}