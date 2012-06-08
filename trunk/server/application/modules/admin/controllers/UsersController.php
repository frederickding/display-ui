<?php
/**
 * UsersController for Admin module
 *
 * Copyright 2011 Frederick Ding<br />
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
 * Functionality for managing users
 */
class Admin_UsersController extends Admin_ControllerAbstract
{
    public function indexAction ()
    {
        $this->_forward('list');
    }
    public function listAction ()
    {
        $UsersModel = new Admin_Model_Users();
        $this->view->usersList = $UsersModel->fetchUsers();
        $this->view->insertForm = $this->insertForm();
        $this->auth_session->deleteCsrf = sha1(microtime(TRUE));
        $this->view->csrf = $this->auth_session->deleteCsrf;
    }
    public function insertForm ()
    {
        $this->view->doctype('XHTML1_STRICT');
        $username = new Zend_Form_Element_Text('username', 
        array('label' => 'Pick a username', 'required' => true));
        $password = new Zend_Form_Element_Password('password', 
        array('label' => 'Enter a password', 'required' => true, 
        'description' => 'Passwords are stored in irreversible hashed form.'));
        $email = new Zend_Form_Element_Text('email', 
        array('label' => 'Enter an e-mail address', 'required' => true));
        $email->addValidator(new Zend_Validate_EmailAddress());
        $aclrole = new Zend_Form_Element_Select('aclrole', 
        array('label' => 'User ACL role', 'required' => true, 
        'description' => 'Admins have access to every part of the system. Publishers can only manage content, and IT manages the technical parts of the system.'));
        $aclrole->addMultiOptions(
        array('publisher' => 'Publisher', 'it' => 'IT', 'admin' => 'Admin'));
        $aclrole->addValidator(
        new Zend_Validate_InArray(
        array('publisher', 'it', 'admin')));
        $yubikey = new Zend_Form_Element_Text('yubikey', 
        array('label' => 'Yubikey public ID', 'required' => false, 
        'description' => 'To require this user to authenticate with a hardware token, type in the first 12 characters of the Yubikey output or press it in this box.'));
        $submit = new Zend_Form_Element_Submit('usersubmit', 
        array('label' => 'Save'));
        $submit->setDecorators(array('ViewHelper'));
        $reset = new Zend_Form_Element_Reset('userreset', 
        array('label' => 'Reset'));
        $reset->setDecorators(array('ViewHelper'));
        $csrf = new Zend_Form_Element_Hash('usercsrf', 
        array('salt' => 'unique'));
        $csrf->removeDecorator('HtmlTag')->removeDecorator('Label');
        $form = new Zend_Form();
        $form->setAction(
        $this->view->url(
        array('module' => 'admin', 'controller' => 'users', 
        'action' => 'insert-process')))
            ->setMethod('post')
            ->setAttrib('id', 'userinsert')
            ->addElements(
        array('username' => $username, 'password' => $password, 
        'email' => $email, 'aclrole' => $aclrole, 'yubikey' => $yubikey, 
        'usercsrf' => $csrf, 'usersubmit' => $submit, 'userreset' => $reset));
        // $form->removeDecorator('HtmlTag');
        if (! $this->Acl->isAllowed($this->auth_session->userRole, 
        $this->reqController, 'insert-process')) {
            $form->clearElements();
            $form->addError(
            'Your role, <code>' . $this->auth_session->userRole .
             '</code>, is not authorized to add users.');
            $form->addDecorator('HtmlTag');
        }
        return $form;
    }
    public function insertProcessAction ()
    {
        if (! $this->getRequest()->isPost()) {
            return $this->_redirect(
            $this->view->serverUrl() . $this->view->url(
            array('module' => 'admin', 'controller' => 'users', 
            'action' => 'list')));
        }
        if ($this->Acl->isAllowed($this->auth_session->userRole, 
        $this->reqController, $this->reqAction)) {
            $form = $this->insertForm();
            $UsersModel = new Admin_Model_Users();
            if ($form->isValid($_POST)) {
                $values = $form->getValues();
                $this->view->success = $UsersModel->insertUser(
                $values['username'], $values['password'], $values['email'], 
                $values['aclrole'], $values['yubikey']);
                return $this->render('insert-process');
            } else {
                $this->view->usersList = $UsersModel->fetchUsers();
                $this->view->insertForm = $form;
                return $this->render('list');
            }
        }
    }
    /**
     * Shows a confirmation page for deletion of an item
     */
    public function deleteAction ()
    {
        $this->view->id = (int) $this->_getParam('id');
        $UsersModel = new Admin_Model_Users();
        $this->auth_session->deleteCsrf = sha1(microtime(TRUE));
        $this->view->csrf = $this->auth_session->deleteCsrf;
    }
    public function deleteProcessAction ()
    {
        $this->_helper->viewRenderer->setNoRender();
        $this->_helper->removeHelper('layout');
        if (/*$this->getRequest()->isPost() &&*/ $this->_getParam('deletecsrf') == $this->auth_session->deleteCsrf) {
            $UsersModel = new Admin_Model_Users();
            $id = (int) $this->_getParam('id', 0);
            if (! $this->Acl->isAllowed($this->auth_session->userRole, 
            $this->reqController, $this->reqAction)) {
                return $this->getResponse()->setBody(
                'Access denied for role ' . $this->auth_session->userRole);
            }
            if ($this->_getParam('deleteconf', 'No!') == 'Yes!') {
                if ($UsersModel->deleteUser($id)) {
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
}
