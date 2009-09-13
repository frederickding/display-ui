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
				'action' => 'index'
			)));
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
		$MediaModel = new Admin_Model_Multimedia();
		$this->view->page = $this->_getParam('page', '1');
		$mediaData = $MediaModel->fetchMultimedia($this->view->page);
		$this->view->mediaTotal = $mediaData[0];
		$this->view->mediaData = $mediaData[1];
		$this->view->mediaRange = array(
			$mediaData[2] ,
			$mediaData[3]
		);
	}
	public function uploadAction ()
	{
		$form = $this->uploadForm();
		$this->view->form = $form;
	}
	public function uploadProcessAction ()
	{
		if (! $this->getRequest()->isPost()) {
			return $this->_redirect($this->view->serverUrl() . $this->view->url(array(
				'module' => 'admin' ,
				'controller' => 'multimedia' ,
				'action' => 'upload'
			)));
		}
		$form = $this->uploadForm();
		if ($form->isValid($_POST)) {
			// assign the values and receive the file
			$values = $form->getValues();
			$MediaModel = new Admin_Model_Multimedia();
			$this->view->id = $MediaModel->insertMultimedia($values);
			return $this->render('upload-process');
		} else {
			$this->view->form = $form;
			return $this->render('upload');
		}
	}
	public function uploadForm ()
	{
		$MediaModel = new Admin_Model_Multimedia();
		$listClients = $MediaModel->fetchClients($this->auth_session->username);
		$this->view->doctype('XHTML1_STRICT');
		$title = new Zend_Form_Element_Text('mediumtitle',
			array(
				'label' => 'Title' ,
				'required' => TRUE
			));
		$title->addValidator('StringLength', FALSE, array(
			1 ,
			63
		));
		$immediateActive = new Zend_Form_Element_Radio('mediumactivatenow',
			array(
				'label' => 'Immediately activate?' ,
				'required' => TRUE
			));
		$immediateActive->addMultiOption('1', 'Yes')->addMultiOption('0', 'No');
		$activates = new Zend_Form_Element_Text('mediumactivation',
			array(
				'label' => 'Start showing on (YYYY-MM-DD)' ,
				'required' => FALSE
			));
		$expires = new Zend_Form_Element_Text('mediumexpiration',
			array(
				'label' => 'Stop showing on (YYYY-MM-DD)' ,
				'required' => TRUE
			));
		$activates->addFilter('Alnum');
		$expires->addFilter('Alnum');
		$hash = new Zend_Form_Element_Hash('uploadnonce',
			array(
				'salt' => 'unique'
			));
		$hash->removeDecorator('label');
		$weight = new Zend_Form_Element_Select('mediumweight',
			array(
				'label' => 'Weight'
			));
		$weight->addValidator('int', FALSE)->addValidator('between', FALSE, array(
			1 ,
			10
		));
		$weight->addMultiOptions(array_combine(range(1, 10), range(1, 10)));
		$submit = new Zend_Form_Element_Submit('mediasubmit',
			array(
				'label' => 'Upload!'
			));
		$submit->setDecorators(array(
			'ViewHelper'
		));
		$reset = new Zend_Form_Element_Reset('mediareset',
			array(
				'label' => 'Reset'
			));
		$reset->setDecorators(array(
			'ViewHelper'
		));
		$file = new Zend_Form_Element_File('mediumfile',
			array(
				'label' => 'Media file' ,
				'required' => TRUE
			));
		$file->addValidator('count', FALSE, 1)->addValidator('Extension', FALSE, array(
			'jpg' ,
			'jpeg' ,
			'png' ,
			'mov' ,
			'mp4' ,
			'avi' ,
			'wmv' ,
			'mkv'
		))->setDestination(MEDIA_DIR);
		$clients = new Zend_Form_Element_Multiselect('mediumclients',
			array(
				'label' => 'Show on these clients' ,
				'required' => TRUE
			));
		foreach ($listClients as $c) {
			$clients->addMultiOption($c['id'], $c['sys_name']);
		}
		// file can be any mime type or extension, we're not checking it
		// form handling part will use the mime type and insert it into the db
		$form = new Zend_Form();
		$form->setAction($this->view->url(array(
			'module' => 'admin' ,
			'controller' => 'multimedia' ,
			'action' => 'upload-process'
		)))->setMethod('post')->setAttrib('id', 'mediaupload')->addElements(array(
			'mediumtitle' => $title ,
			'mediumactivatenow' => $immediateActive ,
			'mediumactivation' => $activates ,
			'mediumexpiration' => $expires ,
			'mediumweight' => $weight ,
			'mediumclients' => $clients ,
			'mediumfile' => $file ,
			'uploadnonce' => $hash ,
			'mediasubmit' => $submit ,
			'mediareset' => $reset
		));
		return $form;
	}
}