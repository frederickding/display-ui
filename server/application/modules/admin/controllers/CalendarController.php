<?php
/**
 * CalendarController for Admin module
 *
 * Copyright 2010 Frederick Ding<br />
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
 * Functionality for managing events
 */
class Admin_CalendarController extends Zend_Controller_Action
{
	public function init ()
	{
		// initiate a session for the installer
		$this->auth_session = new Zend_Session_Namespace('auth');
		// ALWAYS check if authenticated
		if (! $this->auth_session->authenticated) {
			return $this->_redirect(
			$this->view->serverUrl() . $this->view->url(
			array(
				'module' => 'admin',
				'controller' => 'login',
				'action' => 'index')) . '?redirect=' . $this->view->url(
			array(
				'module' => 'admin',
				'controller' => $this->_request->getControllerName(),
				'action' => $this->_request->getActionName())));
		}
		// configuration object
		if (Zend_Registry::isRegistered('configuration_ini')) {
			$config = Zend_Registry::get('configuration_ini');
		} else {
			$config = new Zend_Config_ini(CONFIG_DIR . '/configuration.ini',
			'production');
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
		$CalendarModel = new Admin_Model_Calendar();
		$this->view->eventsList = $CalendarModel->getAllEvents();
		$this->view->insertForm = $this->insertForm();
	}
	/**
	 * Shows a confirmation page for deletion of an item
	 */
	public function deleteAction ()
	{
		$this->view->id = (int) $this->_getParam('id');
		/* $HeadlinesModel = new Admin_Model_Headlines(); */
		$this->auth_session->deleteCrsf = sha1(microtime(TRUE));
		$this->view->csrf = $this->auth_session->deleteCrsf;
		$this->view->canDelete = true;
		/* $this->view->canDelete = $HeadlinesModel->canModify($this->auth_session->username, $this->view->id); */
	}
	public function deleteProcessAction ()
	{
		$this->_helper->viewRenderer->setNoRender();
		$this->_helper->layout()->disableLayout();
		if (/*$this->getRequest()->isPost() &&*/ $this->_getParam('deletecrsf') == $this->auth_session->deleteCsrf) {
			$CalendarModel = new Admin_Model_Calendar();
			$id = (int) $this->_getParam('id', 0);
			if ($this->_getParam('deleteconf', 'No!') == 'Yes!') {
				if ($CalendarModel->deleteEvent($id)) {
					return $this->getResponse()->setBody(
					'Successfully deleted.');
				} else {
					return $this->getResponse()->setBody('Deletion failed.');
				}
			}
		}
		return $this->getResponse()->setBody(
		'Nothing was deleted. <a href="javascript:history.back(1);">Go back.</a>');
	}
	public function insertProcessAction ()
	{
		if (! $this->getRequest()->isPost()) {
			return $this->_redirect(
			$this->view->serverUrl() . $this->view->url(
			array(
				'module' => 'admin',
				'controller' => 'calendar',
				'action' => 'list')));
		}
		$form = $this->insertForm();
		$CalendarModel = new Admin_Model_Calendar();
		if ($form->isValid($_POST)) {
			$values = $form->getValues();
			$this->view->success = $CalendarModel->insertEvent(
			$values['eventname'], $values['eventtime'], $values['eventclient'],
			true, $values['eventtype']);
			return $this->render('insert-process');
		} else {
			$this->view->insertForm = $form;
			$this->view->headlinesList = $CalendarModel->getAllEvents();
			return $this->render('list');
		}
	}
	public function insertForm ()
	{
		$ClientModel = new Admin_Model_Clients();
		$listClients = $ClientModel->fetchClients($this->auth_session->username);
		unset($ClientModel);
		$this->view->doctype('XHTML1_STRICT');
		$name = new Zend_Form_Element_Text('eventname',
		array(
			'label' => 'Event Name',
			'required' => TRUE));
		$name->setAttribs(array(
			'cols' => 50,
			'rows' => 3));
		$show = new Zend_Form_Element_Select('eventclient',
		array(
			'label' => 'Show on',
			'required' => TRUE));
		foreach ($listClients as $c) {
			$show->addMultiOption($c['id'], $c['sys_name']);
		}
		$type = new Zend_Form_Element_Select('eventtype',
		array(
			'label' => 'Event type',
			'required' => TRUE,
			'description' => 'Is this a one-time event or does it repeat weekly?"'));
		$type->addMultiOptions(
		array(
			'once' => 'One time',
			'weekly' => 'Weekly'));
		$time = new Zend_Form_Element_Text('eventtime',
		array(
			'label' => 'Event date/time',
			'required' => TRUE,  // TODO: better local timezone support using CONVERT_TZ() in SQL
			'description' => 'Enter in YYYY-MM-DD HH:MM:SS format. If this is a weekly event, this is the start date.'));
		$time->addFilter('Digits');
		$submit = new Zend_Form_Element_Submit('eventsubmit',
		array(
			'label' => 'Save'));
		$submit->setDecorators(array(
			'ViewHelper'));
		$reset = new Zend_Form_Element_Reset('eventreset',
		array(
			'label' => 'Reset'));
		$reset->setDecorators(array(
			'ViewHelper'));
		$csrf = new Zend_Form_Element_Hash('eventcsrf',
		array(
			'salt' => 'unique'));
		$csrf->removeDecorator('HtmlTag')->removeDecorator('Label');
		$form = new Zend_Form();
		$form->setAction(
		$this->view->url(
		array(
			'module' => 'admin',
			'controller' => 'calendar',
			'action' => 'insert-process')))
			->setMethod('post')
			->setAttrib('id', 'eventinsert')
			->addElements(
		array(
			'eventname' => $name,
			'eventclient' => $show,
			'eventtype' => $type,
			'eventtime' => $time,
			'eventcsrf' => $csrf,
			'eventsubmit' => $submit,
			'eventreset' => $reset));
		// $form->removeDecorator('HtmlTag');
		return $form;
	}
}