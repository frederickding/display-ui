<?php
/**
 * Headlines controller for Admin module
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
 * Functionality for managing headlines
 */
class Admin_HeadlinesController extends Admin_ControllerAbstract
{
	public function indexAction ()
	{
		$this->_forward('list');
	}
	public function listAction ()
	{
		$HeadlinesModel = new Admin_Model_Headlines();
		$this->view->headlinesList = $HeadlinesModel->fetchHeadlines(
		$this->auth_session->username);
		$this->view->insertForm = $this->insertForm();
		$this->auth_session->deleteCsrf = sha1(microtime(TRUE));
		$this->view->csrf = $this->auth_session->deleteCsrf;
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
		$this->view->canDelete = $HeadlinesModel->canModify(
		$this->auth_session->username, $this->view->id);
	}
	public function deleteProcessAction ()
	{
		$this->_helper->viewRenderer->setNoRender();
		$this->_helper->removeHelper('layout');
		if (/*$this->getRequest()->isPost() &&*/ $this->_getParam('deletecsrf') == $this->auth_session->deleteCsrf) {
			$HeadlinesModel = new Admin_Model_Headlines();
			$id = (int) $this->_getParam('id', 0);
			if ($this->_getParam('deleteconf', 'No!') == 'Yes!') {
				if ($HeadlinesModel->deleteHeadline($id)) {
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
				'controller' => 'headlines',
				'action' => 'list')));
		}
		$form = $this->insertForm();
		$HeadlinesModel = new Admin_Model_Headlines();
		if ($form->isValid($_POST)) {
			$values = $form->getValues();
			$this->view->success = ($HeadlinesModel->insertHeadline(
			$values['headlinemessage'], $values['headlineclient'],
			$values['headlinetype'], $values['headlineexpires'], $values['headlinealternating']));
			return $this->render('insert-process');
		} else {
			$this->view->insertForm = $form;
			$this->view->headlinesList = $HeadlinesModel->fetchHeadlines(
			$this->auth_session->username);
			return $this->render('list');
		}
	}
	public function insertForm ()
	{
		$DashboardModel = new Admin_Model_Dashboard();
		$listClients = $DashboardModel->fetchClients(
		$this->auth_session->username);
		unset($DashboardModel);
		$this->view->doctype('XHTML1_STRICT');
		$message = new Zend_Form_Element_Textarea('headlinemessage',
		array(
			'label' => 'Add a headline message',
			'required' => TRUE));
		$message->setAttribs(array(
			'cols' => 50,
			'rows' => 3));
		$show = new Zend_Form_Element_Select('headlineclient',
		array(
			'label' => 'Show on',
			'required' => TRUE));
		foreach ($listClients as $c) {
			$show->addMultiOption($c['id'], $c['sys_name']);
		}
		$type = new Zend_Form_Element_Select('headlinetype',
		array(
			'label' => 'Message type',
			'required' => TRUE,
			'description' => 'Most headlines are news. PSAs are low-priority messages such as "Weather data provided by Yahoo! Weather."'));
		$type->addMultiOptions(
		array(
			'news' => 'News',
			'psa' => 'Public Service Announcement'));
		$alternating = new Zend_Form_Element_Select('headlinealternating',
		array(
			'label' => 'On which days should this play?',
			'required' => true,
			'description' => 'Will this item show only on Day 1, Day 2 or all days?'));
		$alternating->addMultiOptions(array(
			'0' => 'All Days',
			'1' => 'Day 1',
			'2' => 'Day 2'
		));
		$expires = new Zend_Form_Element_Text('headlineexpires',
		array(
			'label' => 'Stop showing on',
			'required' => FALSE,  // TODO: better local timezone support using CONVERT_TZ() in SQL
			'description' => 'Enter in YYYY-MM-DD format. If you use the more specific YYYY-MM-DD HH:MM:SS format, it must be in UTC. If this is blank, the headline will, by default, expire in 1 month.'));
		$expires->addFilter('Digits');
		$submit = new Zend_Form_Element_Submit('headlinesubmit',
		array(
			'label' => 'Save'));
		$submit->setDecorators(array(
			'ViewHelper'));
		$reset = new Zend_Form_Element_Reset('headlinereset',
		array(
			'label' => 'Reset'));
		$reset->setDecorators(array(
			'ViewHelper'));
		$csrf = new Zend_Form_Element_Hash('headlinecsrf',
		array(
			'salt' => 'unique'));
		$csrf->removeDecorator('HtmlTag')->removeDecorator('Label');
		$form = new Zend_Form();
		$form->setAction(
		$this->view->url(
		array(
			'module' => 'admin',
			'controller' => 'headlines',
			'action' => 'insert-process')))
			->setMethod('post')
			->setAttrib('id', 'headlineinsert')
			->addElements(
		array(
			'headlinemessage' => $message,
			'headlineclient' => $show,
			'headlinetype' => $type,
			'headlinealternating' => $alternating,
			'headlineexpires' => $expires,
			'headlinecsrf' => $csrf,
			'headlinesubmit' => $submit,
			'headlinereset' => $reset));
		// $form->removeDecorator('HtmlTag');
		return $form;
	}
}