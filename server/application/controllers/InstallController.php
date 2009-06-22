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
	 * Don't actually do anything
	 */
	public function indexAction ()
	{
		// first of all, we need a way to find the current URL
		// and the URL of the images
		$this->view->base_uri = explode("/install", $_SERVER['REQUEST_URI']);
		$this->view->base_uri = $this->view->base_uri[0];
		$this->view->version = APPLICATION_VER;
		
		// initiate a session for the installer
		$this->session = new Zend_Session_Namespace('installer');
	    $this->session->page = 1;
	}
	/**
	 * Test method
	 *
	 * Checks whether server configuration is compatible
	 */
	public function testAction ()
	{
		$this->view->base_uri = explode("/install", $_SERVER['REQUEST_URI']);
		$this->view->base_uri = $this->view->base_uri[0];
		$this->view->version = APPLICATION_VER;
		$this->view->tests = array();
		
		// if session page isn't 1, don't proceed
		$this->session = new Zend_Session_Namespace('installer');
		if(!($this->session->page >= 1))
			$this->_redirect('http://'.$_SERVER['SERVER_NAME'].$this->view->base_uri.'/install/');
		
		// if PHP version is greater than 5.2.0, TRUE (pass)
		$this->view->tests[0] = phpversion() >= '5.2.0';
		// hash is necessary
		$this->view->tests[1] = extension_loaded('hash');
		// curl?
		$this->view->tests[2] = extension_loaded('curl');
		// sockets?
		$this->view->tests[3] = extension_loaded('sockets');
		// PDO & PDO_MySQL
		$this->view->tests[4] = (extension_loaded('pdo') && extension_loaded('pdo_mysql'));
		// MySQLi?
		$this->view->tests[5] = extension_loaded('mysqli');
		// APC?
		$this->view->tests[6] = extension_loaded('apc');
		// writable files?
		$this->view->tests[7] = (@file_exists(CONFIG_DIR.'/configuration.default.ini')
								&& @file_exists(CONFIG_DIR.'/database.default.ini')
								&& is_writable(CONFIG_DIR));

		// Safe mode off
		$this->view->tests[8] = ((bool) ini_get('safe_mode')) ? FALSE : TRUE;
		// Output buffering
		$this->view->tests[9] = ((bool) ini_get('output_buffering')) ? FALSE : TRUE;
		
		$this->session->page = 2;
	}
	/**
	 * Configuration set up method
	 *
	 * Reads the distrib configuration files and changes certain variables,
	 * then writes those files to the configuration file.
	 */
	public function configAction ()
	{
		$this->view->base_uri = explode("/install", $_SERVER['REQUEST_URI']);
		$this->view->base_uri = $this->view->base_uri[0];
		$this->view->version = APPLICATION_VER;
		
		// if session page isn't 1, don't proceed
		$this->session = new Zend_Session_Namespace('installer');
		if($this->session->page != 2)
			$this->_redirect('http://'.$_SERVER['SERVER_NAME'].$this->view->base_uri.'/install/');
		
		$this->_helper->viewRenderer->setNoRender();
		if (file_exists(CONFIG_DIR . '/configuration.ini')) {
			// the configuration is already set up
			$this->view->h1 = 'An error occurred';
			$this->view->title = 'System already set up';
			$this->view->message = 'According to the system, '
				. 'the configuration has already been set up. '
				. 'The installer will not continue.';
			$this->render('config');
		} else {
			// we need to set up the configuration file
			$config = new Zend_Config_Ini(CONFIG_DIR . '/configuration.default.ini',
				null, array(
				'allowModifications' => true
			));
			// note the date of installation
			$config->production->server->install->date = date('Y-m-d');
			/*
			 * generate a reasonably random secret
			 * that must NEVER be changed after install
			 * or cryptographic hashes generated with it will become invalid!
			 */
			$config->production->server->install->secret = sha1(
				hash_hmac('sha256', time(), sha1(microtime(TRUE))));
			// now write the new config to file
			$writer = new Zend_Config_Writer_Ini(array(
				'config' => $config ,
				'filename' => CONFIG_DIR . '/configuration.ini'
			));
			$writer->write();
			$this->view->h1 = 'Install successful';
			$this->view->title = 'Display UI Server is ready to go';
			$this->view->message = 'The system has set up your configuration files.'
				. '<br />'
				. 'Keep this server secret in a safe place:<br />'
				. '<code>' . $config->production->server->install->secret . '</code>';
			$this->render('config');
		}
	}
}