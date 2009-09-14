<?php
/**
 * Headlines controller for Admin module
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
 * Functionality for managing headlines
 */
class Admin_HeadlinesController extends Zend_Controller_Action
{
	public function init ()
	{
		// initiate a session for the installer
		$this->auth_session = new Zend_Session_Namespace('auth');
		// ALWAYS check if authenticated
		if (! $this->auth_session->authenticated) {
			return $this->_redirect($this->view->serverUrl() . $this->view->url(array(
				'module' => 'admin' ,
				'controller' => 'login' ,
				'action' => 'index'
			)));
		}
			// configuration object
		if(Zend_Registry::isRegistered('configuration_ini')) {
			$config = Zend_Registry::get('configuration_ini');
		} else {
			$config = new Zend_Config_ini(CONFIG_DIR . '/configuration.ini', 'production');
			Zend_Registry::set('configuration_ini', $config);
		}
		$this->view->systemName = $config->server->install->name;
		$this->view->username = $this->auth_session->username;
		$this->_helper->layout()->setLayout('AdminPanelWidgets');
		return TRUE;
	}
	public function indexAction ()
	{
		$this->_forward('list');
	}
	public function listAction ()
	{
		$HeadlinesModel = new Admin_Model_Headlines();
		$this->view->headlinesList = $HeadlinesModel->fetchHeadlines($this->auth_session->username);
	}
	/**
	 * Shows a confirmation page for deletion of an item
	 */
	public function deleteAction ()
	{
		$this->view->id = (int) $this->_getParam('id');
		$HeadlinesModel = new Admin_Model_Headlines();
		$this->auth_session->deleteCrsf = sha1(microtime(TRUE));
		$this->view->csrf = $this->auth_session->deleteCrsf;
		$this->view->canDelete = $HeadlinesModel->canModify($this->auth_session->username, $this->view->id);
	}
	public function deleteProcessAction ()
	{
		$this->_helper->viewRenderer->setNoRender();
		$this->_helper->removeHelper('layout');
		if (/*$this->getRequest()->isPost() &&*/ $this->_getParam('deletecrsf') == $this->auth_session->deleteCsrf) {
			$HeadlinesModel = new Admin_Model_Headlines();
			$id = (int) $this->_getParam('id', 0);
			if ($this->_getParam('deleteconf', 'No!') == 'Yes!') {
				if ($HeadlinesModel->deleteHeadline($id)) {
					return $this->getResponse()->setBody('Successfully deleted.');
				} else {
					return $this->getResponse()->setBody('Deletion failed.');
				}
			}
		}
		return $this->getResponse()->setBody('Nothing was deleted. <a href="javascript:history.back(1);">Go back.</a>');
	}
}