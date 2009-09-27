<?php
/**
 * Media for Admin module
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
 * Functionality for multimedia
 */
class Admin_MultimediaController extends Zend_Controller_Action
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
				'action' => 'index')));
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
	/**
	 * The default action for this module, internally forwards to listAction
	 */
	public function indexAction ()
	{
		$this->_forward('list');
	}
	/**
	 * Shows a listing of the media items in the media table
	 */
	public function listAction ()
	{
		$MediaModel = new Admin_Model_Multimedia();
		$this->view->page = $this->_getParam('page', '1');
		$mediaData = $MediaModel->fetchMultimedia($this->view->page);
		$this->view->mediaTotal = $mediaData[0];
		$this->view->mediaData = $mediaData[1];
		$this->view->mediaRange = array(
			$mediaData[2] ,
			$mediaData[3]);
	}
	/**
	 * Shows a form for uploading media into the system
	 */
	public function uploadAction ()
	{
		$form = $this->uploadForm();
		$this->view->form = $form;
	}
	/**
	 * Processes the upload form
	 */
	public function uploadProcessAction ()
	{
		if (! $this->getRequest()->isPost()) {
			return $this->_redirect($this->view->serverUrl() . $this->view->url(array(
				'module' => 'admin' ,
				'controller' => 'multimedia' ,
				'action' => 'upload')));
		}
		$form = $this->uploadForm();
		if ($form->isValid($_POST)) {
			// assign the values and receive the file
			// FIXME: MASSIVELY BROKEN OVERWRITING!
			$values = $form->getValues();
			$MediaModel = new Admin_Model_Multimedia();
			$this->view->id = $MediaModel->insertMultimedia($values);
			return $this->render('upload-process');
		} else {
			$this->view->form = $form;
			return $this->render('upload');
		}
	}
	/**
	 * Shows a confirmation page for deletion of an item
	 */
	public function deleteAction ()
	{
		$this->view->id = (int) $this->_getParam('id');
		$MediaModel = new Admin_Model_Multimedia();
		$this->auth_session->deleteCrsf = sha1(microtime(TRUE));
		$this->view->csrf = $this->auth_session->deleteCrsf;
		$this->view->canDelete = $MediaModel->canModify($this->auth_session->username, $this->view->id);
	}
	public function deleteProcessAction ()
	{
		$this->_helper->viewRenderer->setNoRender();
		$this->_helper->removeHelper('layout');
		if (/*$this->getRequest()->isPost() &&*/ $this->_getParam('deletecrsf') == $this->auth_session->deleteCsrf) {
			$MediaModel = new Admin_Model_Multimedia();
			$id = (int) $this->_getParam('id', 0);
			if ($this->_getParam('deleteconf', 'No!') == 'Yes!') {
				if ($MediaModel->deleteMultimedia($id)) {
					return $this->getResponse()->setBody('Successfully deleted.');
				} else {
					return $this->getResponse()->setBody('Deletion failed.');
				}
			}
		}
		return $this->getResponse()->setBody('Nothing was deleted. <a href="javascript:history.back(1);">Go back.</a>');
	}
	public function viewAction ()
	{
		$MediaModel = new Admin_Model_Multimedia();
		$this->view->id = (int) $this->_getParam('id', 0);
		$query = $MediaModel->getMedium($this->view->id);
		if ($query->type == Admin_Model_Multimedia::IMAGE_TYPE) {
			$this->view->canShow = TRUE;
			$this->view->fileBinary = $query->fileBinary;
			$this->view->mimeType = $query->mime;
		} else {
			$this->view->canShow = FALSE;
			$this->view->type = $query->type;
		}
	}
	/**
	 * Produces the Zend_Form that is used for upload functionality
	 * @return Zend_Form
	 */
	public function uploadForm ()
	{
		$MediaModel = new Admin_Model_Multimedia();
		$listClients = $MediaModel->fetchClients($this->auth_session->username);
		unset($MediaModel);
		$this->view->doctype('XHTML1_STRICT');
		$title = new Zend_Form_Element_Text('mediumtitle',
			array(
				'label' => 'Title' ,
				'required' => TRUE));
		$title->addValidator('StringLength', FALSE, array(
			1 ,
			63));
		$immediateActive = new Zend_Form_Element_Radio('mediumactivatenow',
			array(
				'label' => 'Immediately activate?' ,
				'required' => TRUE));
		$immediateActive->addMultiOption('1', 'Yes')->addMultiOption('0', 'No')->setDescription('Will this item begin showing immediately or at a later date?');
		$activates = new Zend_Form_Element_Text('mediumactivation',
			array(
				'label' => 'Start showing on (YYYY-MM-DD)' ,
				'required' => FALSE));
		$expires = new Zend_Form_Element_Text('mediumexpiration',
			array(
				'label' => 'Stop showing on (YYYY-MM-DD)' ,
				'required' => FALSE));
		$activates->addFilter('Alnum');
		$expires->addFilter('Alnum');
		$duration = new Zend_Form_Element_Text('mediumduration',
			array(
				'label' => 'Length to show image'));
		$duration->setValue('15')->setDescription('This value is ignored for videos.');
		$weight = new Zend_Form_Element_Select('mediumweight',
			array(
				'label' => 'Weight'));
		$weight->addValidator('int', FALSE)->addValidator('between', FALSE, array(
			1 ,
			10))->setDescription('A higher weight usually results in the file showing more frequently.')->addMultiOptions(array_combine(range(1, 10), range(1, 10)));
		$submit = new Zend_Form_Element_Submit('mediasubmit',
			array(
				'label' => 'Upload!'));
		$submit->setDecorators(array(
			'ViewHelper'));
		$reset = new Zend_Form_Element_Reset('mediareset',
			array(
				'label' => 'Reset'));
		$reset->setDecorators(array(
			'ViewHelper'));
		$file = new Zend_Form_Element_File('mediumfile',
			array(
				'label' => 'Media file' ,
				'required' => TRUE));
		$file->addValidator('count', FALSE, 1)->addValidator('Extension', FALSE, array(
			'jpg' ,
			'jpeg' ,
			'png' ,
			'mov' ,
			'mp4' ,
			'avi' ,
			'wmv' ,
			'mkv'))->setDestination(MEDIA_DIR)->setDescription('Upload a file in a supported format (JPEG, PNG, MP4, MOV, AVI, MKV, WMV).');
		$clients = new Zend_Form_Element_Multiselect('mediumclients',
			array(
				'label' => 'Show on these clients' ,
				'required' => TRUE));
		$clients->setDescription('You can hold down Shift or Ctrl to select multiple clients.');
		foreach ($listClients as $c) {
			$clients->addMultiOption($c['id'], $c['sys_name']);
		}
		// file can be any mime type or extension, we're not checking it
		// form handling part will use the mime type and insert it into the db
		$form = new Zend_Form();
		$form->setAction($this->view->url(array(
			'module' => 'admin' ,
			'controller' => 'multimedia' ,
			'action' => 'upload-process')))->setMethod('post')->setAttrib('id', 'mediaupload')->addElements(array(
			'mediumtitle' => $title ,
			'mediumclients' => $clients ,
			'mediumfile' => $file ,
			'mediumactivatenow' => $immediateActive ,
			'mediumactivation' => $activates ,
			'mediumexpiration' => $expires ,
			'mediumweight' => $weight ,
			'mediumduration' => $duration ,
			'mediasubmit' => $submit ,
			'mediareset' => $reset));
		return $form;
	}
}