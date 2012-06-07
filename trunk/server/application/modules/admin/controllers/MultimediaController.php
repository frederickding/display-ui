<?php
/**
 * Media for Admin module
 *
 * Copyright 2009-2012 Frederick Ding<br />
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
 * Functionality for multimedia
 */
class Admin_MultimediaController extends Admin_ControllerAbstract
{
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
			$mediaData[2], 
			$mediaData[3]);
		$this->auth_session->deleteCrsf = sha1(microtime(TRUE));
		$this->view->csrf = $this->auth_session->deleteCsrf;
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
			return $this->_redirect(
			$this->view->serverUrl() . $this->view->url(
			array(
				'module' => 'admin', 
				'controller' => 'multimedia', 
				'action' => 'upload')));
		}
		if ($this->Acl->isAllowed($this->auth_session->userRole, 
		$this->reqController, $this->reqAction)) {
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
	}
	public function toggleAction ()
	{
		$id = $this->_getParam('id');
		$MediaModel = new Admin_Model_Multimedia();
		if (! is_null($id) &&
		 $MediaModel->canModify($this->auth_session->username, $id)) {
			$MediaModel->toggleActive($id);
		}
		// redir back to list
		return $this->_redirect(
		$this->view->serverUrl() . $this->view->url(
		array(
			'module' => 'admin', 
			'controller' => 'multimedia', 
			'action' => 'index')));
	}
	/**
	 * Shows a confirmation page for deletion of an item
	 */
	public function deleteAction ()
	{
		$this->view->id = (int) $this->_getParam('id');
		$MediaModel = new Admin_Model_Multimedia();
		$this->auth_session->deleteCrsf = sha1(microtime(true));
		$this->view->csrf = $this->auth_session->deleteCrsf;
		$this->view->canDelete = $MediaModel->canModify(
		$this->auth_session->username, $this->view->id);
	}
	public function deleteProcessAction ()
	{
		$this->_helper->viewRenderer->setNoRender();
		$this->_helper->removeHelper('layout');
		if (/*$this->getRequest()->isPost() &&*/ $this->_getParam('deletecrsf') == $this->auth_session->deleteCsrf) {
			$MediaModel = new Admin_Model_Multimedia();
			$id = (int) $this->_getParam('id', 0);
			if (! $this->Acl->isAllowed($this->auth_session->userRole, 
			$this->reqController, $this->reqAction)) {
				return $this->getResponse()->setBody(
				'Access denied for role ' . $this->auth_session->userRole);
			}
			if ($this->_getParam('deleteconf', 'No!') == 'Yes!') {
				if ($MediaModel->deleteMultimedia($id)) {
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
	public function viewAction ()
	{
		$MediaModel = new Admin_Model_Multimedia();
		$this->view->id = (int) $this->_getParam('id', 0);
		$query = $MediaModel->getMedium($this->view->id);
		if ($query->type == Admin_Model_Multimedia::IMAGE_TYPE) {
			$this->view->canShow = true;
			$this->view->fileBinary = $query->fileBinary;
			$this->view->mimeType = $query->mime;
		} else {
			$this->view->canShow = false;
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
			'label' => 'Title', 
			'required' => true));
		$title->addValidator('StringLength', false, array(
			1, 
			63));
		$immediateActive = new Zend_Form_Element_Radio('mediumactivatenow', 
		array(
			'label' => 'Immediately activate?', 
			'required' => true, 
			'value' => '1', 
			'description' => 'Will this item begin showing immediately or at a later date?'));
		$immediateActive->addMultiOption('1', 'Yes')->addMultiOption('0', 'No');
		$alternating = new Zend_Form_Element_Select('mediumalternating', 
		array(
			'label' => 'On which days should this play?', 
			'required' => true, 
			'description' => 'Will this item show only on Day 1, Day 2 or all days?'));
		$alternating->addMultiOptions(
		array(
			'0' => 'All Days', 
			'1' => 'Day 1', 
			'2' => 'Day 2'));
		$activates = new Zend_Form_Element_Text('mediumactivation', 
		array(
			'label' => 'Start showing on (YYYY-MM-DD)', 
			'required' => false));
		$expires = new Zend_Form_Element_Text('mediumexpiration', 
		array(
			'label' => 'Stop showing on (YYYY-MM-DD)', 
			'required' => false));
		$activates->addFilter('Digits');
		$expires->addFilter('Digits');
		$duration = new Zend_Form_Element_Text('mediumduration', 
		array(
			'label' => 'Length to show image', 
			'description' => 'This value is ignored for videos.'));
		$duration->setValue('15');
		$weight = new Zend_Form_Element_Select('mediumweight', 
		array(
			'label' => 'Weight', 
			'description' => 'A higher weight usually results in the file showing more frequently.'));
		$weight->addValidator('int', false)
			->addValidator('between', false, array(
			1, 
			10))
			->addMultiOptions(array_combine(range(1, 10), range(1, 10)));
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
			'label' => 'Media file', 
			'required' => true, 
			'description' => 'Upload a file in a supported format (JPEG, PNG, MP4, MOV, AVI, MKV, WMV, MPG, MPEG.'));
		$file->addValidator('count', false, 1)
			->addValidator('Extension', false, 
		array(
			'jpg', 
			'jpeg', 
			'png', 
			'mov', 
			'mp4', 
			'avi', 
			'wmv', 
			'mkv', 
			'zip', 
			'mpg', 
			'mpeg', 
			'duizip'))
			->setDestination(MEDIA_DIR);
		$clients = new Zend_Form_Element_Multiselect('mediumclients', 
		array(
			'label' => 'Show on these clients', 
			'required' => true, 
			'description' => 'You can hold down Shift or Ctrl to select multiple clients.'));
		foreach ($listClients as $c) {
			$clients->addMultiOption($c['id'], $c['sys_name']);
		}
		// file can be any mime type or extension, we're not checking it
		// form handling part will use the mime type and insert it into the db
		$form = new Zend_Form();
		$form->setAction(
		$this->view->url(
		array(
			'module' => 'admin', 
			'controller' => 'multimedia', 
			'action' => 'upload-process')))
			->setMethod('post')
			->setAttrib('id', 'mediaupload')
			->addElements(
		array(
			'mediumtitle' => $title, 
			'mediumclients' => $clients, 
			'mediumfile' => $file, 
			'mediumactivatenow' => $immediateActive, 
			'mediumactivation' => $activates, 
			'mediumalternating' => $alternating, 
			'mediumexpiration' => $expires, 
			'mediumweight' => $weight, 
			'mediumduration' => $duration, 
			'mediasubmit' => $submit, 
			'mediareset' => $reset));
		return $form;
	}
}