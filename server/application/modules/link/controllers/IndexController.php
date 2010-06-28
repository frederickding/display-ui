<?php
/**
 * Link module index controller
 *
 * Copyright 2010 Frederick Ding<br />
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
class Link_IndexController extends Zend_Controller_Action
{
	public function init ()
	{
		// initiate a session for authorization
		$this->auth_session = new Zend_Session_Namespace('auth');
		// ALWAYS check if authenticated
		if (! $this->auth_session->authenticated) {
			return $this->_redirect($this->view->serverUrl() . $this->view->url(array(
				'module' => 'admin' ,
				'controller' => 'login' ,
				'action' => 'index')) . '?redirect=' . $this->view->url(array(
				'module' => 'link' ,
				'controller' => $this->_request->getControllerName() ,
				'action' => $this->_request->getActionName())));
		}

		// ACL
		if (! Zend_Registry::isRegistered('dui_acl')) {
			$this->acl = new Admin_Model_Acl();
			Zend_Registry::set('dui_acl', $this->acl);
		} else {
			$this->acl = Zend_Registry::get('dui_acl');
		}
		$this->auth = new Admin_Model_Authentication();
		$this->user_role = $this->auth->userRole($this->auth_session->username);
		if (! $this->acl->isAllowed($this->user_role, 'clients', 'link') && $this->_request->getActionName() != 'unauthorized') {
			$this->_forward('unauthorized');
		}

		// configuration object
		if (Zend_Registry::isRegistered('configuration_ini')) {
			$config = Zend_Registry::get('configuration_ini');
		} else {
			$config = new Zend_Config_ini(CONFIG_DIR . '/configuration.ini',
				'production');
			Zend_Registry::set('configuration_ini', $config);
		}

		$this->clients_model = new Admin_Model_Clients();
	}
	/**
	 * The default action
	 */
	public function indexAction ()
	{
		$this->_helper->layout()->setLayout('SimpleBlue');
		$client_auth = new Api_Model_Authenticator();
		$list_clients = $this->clients_model->fetchClients($this->auth_session->username);
		$this->view->api_keys = array();
		for ($i = 0; $i < count($list_clients); $i ++) {
			$this->view->api_keys[$i][0] = $list_clients[$i]['sys_name'];
			$this->view->api_keys[$i][1] = $client_auth->apiKey($list_clients[$i]['sys_name']);
		}
	}
	public function unauthorizedAction ()
	{
		$this->_helper->layout()->setLayout('SimpleBlue');
		$this->view->assign('user_role', $this->user_role);
	}
}

