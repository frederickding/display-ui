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
	public function init ()
	{
		// initiate a session for the installer
		$this->auth_session = new Zend_Session_Namespace('auth');
		$config = new Zend_Config_Ini(CONFIG_DIR . '/configuration.ini', 'production');
		$this->view->systemName = $config->server->install->name;
		$this->view->username = $this->auth_session->username;
	}
	public function indexAction ()
	{
		// TODO implement actions
		$this->_helper->viewRenderer->setNoRender();
		if (! $this->auth_session->authenticated) {
			$this->_redirect($this->view->serverUrl() . $this->view->url(array(
				'module' => 'admin' ,
				'controller' => 'login' ,
				'action' => 'index'
			)));
		}
	}
	public function dashboardAction ()
	{
		if (! $this->auth_session->authenticated) {
			$this->_redirect($this->view->serverUrl() . $this->view->url(array(
				'module' => 'admin' ,
				'controller' => 'login' ,
				'action' => 'index'
			)));
		}
		$this->_helper->layout()->setLayout('AdminPanelWidgets');
		$DashboardModel = new Admin_Model_Dashboard();
		$this->view->statusReport = $DashboardModel->fetchStatusReport();
		$this->view->activeClients = $DashboardModel->fetchActiveClients();
		$this->listClients = $DashboardModel->fetchClients($this->auth_session->username);
		$this->view->quicklineForm = $this->quicklineForm();
	}
	public function quicklineAction ()
	{
		if (! $this->auth_session->authenticated) {
			return $this->_redirect($this->view->serverUrl() . $this->view->url(array(
				'module' => 'admin' ,
				'controller' => 'login' ,
				'action' => 'index'
			)));
		}
		if (! $this->getRequest()->isPost()) {
			return $this->_redirect($this->view->serverUrl() . $this->view->url(array(
				'module' => 'admin' ,
				'controller' => 'index' ,
				'action' => 'dashboard'
			)));
		}
		$DashboardModel = new Admin_Model_Dashboard();
		$this->listClients = $DashboardModel->fetchClients($this->auth_session->username);
		$this->_helper->viewRenderer->setNoRender();

		$form = $this->quicklineForm();
		$values = $form->getValues();
		if (! $form->isValid($_POST)) {
			return $this->getResponse()->setHttpResponseCode(400);
		}
		// success!
		// process the data by inserting it into the DB
		$DashboardModel->insertQuickline($values['quicklinemessage'], $values['quicklineclients']);
		return $this->getResponse()->setHttpResponseCode(200);
	}
	public function quicklineForm ()
	{
		$message = new Zend_Form_Element_Textarea('quicklinemessage', array(
			'label' => 'Headline Message' ,
			'required' => TRUE
		));
		$message->setAttribs(array(
			'id' => 'quickline-message' ,
			'cols' => 20,
			'rows' => 5
		))->setDecorators(array(
			'ViewHelper' ,
			'Label' ,
			array(
				'HtmlTag' ,
				array(
					'tag' => 'p'
				)
			)
		));
		$show = new Zend_Form_Element_Select('quicklineclients', array(
			'label' => 'Show on'
		));
		$show->setAttribs(array(
			'id' => 'quickline-clients' ,
			'size' => 1
		))->setDecorators(array(
			'ViewHelper' ,
			'Errors' ,
			'Label' ,
			array(
				'HtmlTag' ,
				array(
					'tag' => 'p'
				)
			)
		));
		foreach($this->listClients as $c) {
			$show->addMultiOption($c['id'], $c['sys_name']);
		}
		$submit = new Zend_Form_Element_Submit('quicklinesubmit', array(
			'label' => 'Save & Activate'
		));
		$submit->setAttrib('id', 'quickline-submit')->addDecorator('HtmlTag', array(
			'tag' => 'p'
		))->removeDecorator('DtDdWrapper');
		$form = new Zend_Form();
		$form->setAction($this->view->url(array(
			'module' => 'admin' ,
			'controller' => 'index' ,
			'action' => 'quickline'
		)))->setMethod('post')->setAttrib('id', 'quickline-form')->addElements(array(
			'quicklinemessage' => $message ,
			'quicklineclients' => $show ,
			'quicklinesubmit' => $submit
		))->removeDecorator('DtDdWrapper');
		$form->removeDecorator('HtmlTag');
		return $form;
	}
}
