<?php
/**
 * Installation controller
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
 * Sets up the installation for the first time
 */
class InstallController extends Zend_Controller_Action
{
	/**
	 * Default method
	 *
	 */
	public function indexAction()
	{
		$this->_helper->viewRenderer->setNoRender();
	}
	/**
	 * Configuration set up method
	 *
	 * Reads the distrib configuration files and changes certain variables,
	 * then writes those files to the configuration file.
	 */
	public function configAction ()
	{
		$this->_helper->viewRenderer->setNoRender();
		if (file_exists(APPLICATION_PATH . '/configs/configuration.ini')) {
			// the configuration is already set up
			$this->view->h1 = 'An error occurred';
			$this->view->title = 'System already set up';
			$this->view->message = 'According to the system, the configuration ' . 'has already been set up. The installer will not continue.';
			$this->render('index');
		} else {
			// we need to set up the configuration file
			$config = new Zend_Config_Ini(APPLICATION_PATH . '/configs/configuration.default.ini', null, array(
				'allowModifications' => true
			));
			$config->production->server->install->date = date('Y-m-d');
			$config->production->server->install->secret = sha1(
				hash_hmac('sha256', time(), sha1(microtime(TRUE)))
			);
			$writer = new Zend_Config_Writer_Ini(array(
				'config' => $config ,
				'filename' => APPLICATION_PATH . '/configs/configuration.ini'
			));
			$writer->write();
			$this->view->h1 = 'Install successful';
			$this->view->title = 'Display UI Server is ready to go';
			$this->view->message = 'The system has set up your configuration files.';
			$this->render('index');
		}
	}
}