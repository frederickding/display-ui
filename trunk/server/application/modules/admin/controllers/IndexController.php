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
		$this->view->listClients = $DashboardModel->fetchClients($this->auth_session->username);
		$this->view->quicklineForm = $this->quicklineForm();
	}
	public function quicklineForm ()
	{
		$message = new Zend_Form_Element_Textarea('quickline-message', array(
			'label' => 'Headline Message' ,
			'required' => TRUE
		));
		$message->setAttribs(array(
			'id' => 'quickline-message' ,
			'cols' => 20 ,
			'rows' => 5
		))->addDecorators(array(
			'Label' ,
			new Zend_Form_Decorator_HtmlTag(array(
				'tag' => 'p'
			))
		))->removeDecorator('DtDdWrapper');
		$show = new Zend_Form_Element_Select('quickline-clients', array(
			'label' => 'Show on'
		));
		$show->setAttribs(array(
			'id' => 'quickline-clients' ,
			'size' => 1
		))->addMultiOption('devtesting', 'devtesting')->addDecorators(array(
			'Label' ,
			new Zend_Form_Decorator_HtmlTag(array(
				'tag' => 'p'
			))
		))->removeDecorator('DtDdWrapper');
		$submit = new Zend_Form_Element_Submit('quickline-submit', array(
			'label' => 'Save & Activate'
		));
		$submit->setAttrib('id', 'quickline-submit')->addDecorator(new Zend_Form_Decorator_HtmlTag(array(
			'tag' => 'p'
		)))->removeDecorator('DtDdWrapper');
		$form = new Zend_Form();
		$form->setAction($this->view->url(array(
			'module' => 'admin' ,
			'controller' => 'index' ,
			'action' => 'dashboard'
		)))->setMethod('post')->setAttrib('id', 'quickline-form')->addElements(array(
			'quickline-message' => $message ,
			'quickline-clients' => $show ,
			'quickline-submit' => $submit
		))->removeDecorator('DtDdWrapper');
		return $form;
	}
}