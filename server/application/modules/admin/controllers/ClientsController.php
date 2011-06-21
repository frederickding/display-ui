<?php
/**
 * ClientsController for Admin module
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
require 'ControllerAbstract.php';
/**
 * Functionality for managing clients
 */
class Admin_ClientsController extends Admin_ControllerAbstract
{
	public function indexAction ()
	{
		$this->_forward('list');
	}
	public function listAction ()
	{
		$ClientsModel = new Admin_Model_Clients();
		$this->view->clientsList = $ClientsModel->fetchClients(
		$this->auth_session->username);
	}
	public function editUsersAction ()
	{
		$ClientsModel = new Admin_Model_Clients();
		$_id = $this->_getParam('id');
		if ($this->getRequest()->isPost()) {
			$form = $this->usersForm();
			if ($form->isValid($_POST)) {
				$values = $form->getValues();
				$this->view->success = $ClientsModel->updateUsers($values['id'],
				$values['users']);
				return $this->_helper->redirector->goToSimpleAndExit('index',
				'clients', 'admin', array(
					'id' => null));
			} else {
				$this->view->client = $ClientsModel->fetchClient($_id,
				$this->auth_session->username);
				$this->view->form = $form;
			}
		} else {
			$this->view->client = $ClientsModel->fetchClient($_id,
			$this->auth_session->username);
			$this->view->form = $this->usersForm($this->view->client['users']);
		}
	}
	public function usersForm ($list = '')
	{
		$this->view->doctype('XHTML1_STRICT');
		$users = new Zend_Form_Element_Text('users',
		array(
			'label' => 'Accounts that should have access',
			'required' => false,
			'description' => 'Enter a comma-delimited list of all user IDs that should have access to this client.'));
		if (! empty($list)) {
			$users->setValue($list);
		}
		$id = new Zend_Form_Element_Hidden('id',
		array(
			'required' => true,
			'value' => $this->_getParam('id')));
		$id->removeDecorator('HtmlTag')->removeDecorator('Label');
		$submit = new Zend_Form_Element_Submit('usersubmit',
		array(
			'label' => 'Save'));
		$submit->setDecorators(array(
			'ViewHelper'));
		$csrf = new Zend_Form_Element_Hash('usercsrf',
		array(
			'salt' => 'unique'));
		$csrf->removeDecorator('HtmlTag')->removeDecorator('Label');
		$form = new Zend_Form();
		$form->setAction(
		$this->view->url(
		array(
			'module' => 'admin',
			'controller' => 'clients',
			'action' => 'edit-users',
			'id' => null)))
			->setMethod('post')
			->setAttrib('id', 'editusers')
			->addElements(
		array(
			'users' => $users,
			'id' => $id,
			'usercsrf' => $csrf,
			'usersubmit' => $submit));
		$form->removeDecorator('HtmlTag');
		return $form;
	}
}
