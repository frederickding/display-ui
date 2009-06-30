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
		// MySQLi?
		$this->view->tests[4] = extension_loaded('mysqli');
		// PDO & PDO_MySQL
		$this->view->tests[5] = (extension_loaded('pdo') && extension_loaded('pdo_mysql'));
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
		
		if(!$this->view->tests[0] || !$this->view->tests[1] ||
			!$this->view->tests[8] || !$this->view->tests[9]) {
			$this->view->fail = 100;
		} elseif(!$this->view->tests[2] XOR !$this->view->tests[3]) {
			$this->view->fail = 2;
			$this->session->page = 2;
		} elseif(!$this->view->tests[2] AND !$this->view->tests[3]) {
			$this->view->fail = 200;
		} elseif(!$this->view->tests[4] XOR !$this->view->tests[5]) {
			$this->view->fail = 3;
			$this->session->page = 2;
		} elseif(!$this->view->tests[4] AND !$this->view->tests[5]) {
			$this->view->fail = 300;
		} else {
			$this->view->fail = 0;
			$this->session->page = 2;
		}
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
		
		// if session page isn't 2, don't proceed
		$this->session = new Zend_Session_Namespace('installer');
		if($this->session->page < 2)
			$this->_redirect('http://'.$_SERVER['SERVER_NAME'].$this->view->base_uri.'/install/');
		
		$this->_helper->viewRenderer->setNoRender();
		if (file_exists(CONFIG_DIR . '/configuration.ini')) {
			// the configuration is already set up
			$this->view->status = -1;
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
			// set live flag to true
			$config->production->server->install->live = 1;
			// now write the new config to file
			$writer = new Zend_Config_Writer_Ini(array(
				'config' => $config ,
				'filename' => CONFIG_DIR . '/configuration.ini'
			));
			$writer->write();
			$this->view->status = 1;
			$this->view->secret = $config->production->server->install->secret;
			// let us move on to step 4!
			$this->session->page = 3;
			$this->render('config');
		}
	}
	/**
	 * Sets up the database connection and table structure
	 * (this method does the heaviest work in the installer)
	 */
	public function databaseAction()
	{
		$this->view->base_uri = explode("/install", $_SERVER['REQUEST_URI']);
		$this->view->base_uri = $this->view->base_uri[0];
		$this->view->version = APPLICATION_VER;
		// if session page isn't 3, don't proceed
		$this->session = new Zend_Session_Namespace('installer');
		if($this->session->page < 3)
			$this->_redirect('http://'.$_SERVER['SERVER_NAME'].$this->view->base_uri.'/install/');
		$this->_helper->viewRenderer->setNoRender();
		if (file_exists(CONFIG_DIR . '/database.ini')) {
			// the configuration is already set up
			$this->view->status = -1;
			$this->render('databaseerror');
		} else {
			$req = $this->getRequest();
			if(is_null($req->getParam('driver')))
				$this->render('databaseform');
			else {
				// load all the parameters
				$this->view->hostname = $req->getParam('hostname');
				$this->view->username = $req->getParam('username');
				$this->view->password = $req->getParam('password');
				$this->view->dbname = $req->getParam('dbname');
				$this->view->driver = $req->getParam('driver');
				// instantiate the database install model
				$db = new Default_Model_Installsql(
					false,
					$this->view->hostname,
					$this->view->username,
					$this->view->password,
					$this->view->dbname,
					$this->view->driver
				);
				if($db->test()!=1) {
					// connection is no good
					$this->view->uhoh = TRUE;
					$this->render('databaseform');					
				} elseif($db->installStructure()) { 
					// connection works and structure has been set up
					// we can write database connection to file
					$db->writeConfig();
					// send user to next step
					$this->session->page = 4;
					$this->_redirect('http://'.$_SERVER['SERVER_NAME'].$this->view->base_uri.'/user/');
				} else {
					// connection apparently works but couldn't execute SQL
					$this->view->status = 1;
					$this->render('databaseerror');
				}
			}
		}
	}
	/**
	 * Sets up the first and highest-level user
	 */
	public function userAction()
	{
		$this->view->base_uri = explode("/install", $_SERVER['REQUEST_URI']);
		$this->view->base_uri = $this->view->base_uri[0];
		$this->view->version = APPLICATION_VER;
		// if session page isn't 4, don't proceed
		$this->session = new Zend_Session_Namespace('installer');
		if($this->session->page < 4)
			$this->_redirect('http://'.$_SERVER['SERVER_NAME'].$this->view->base_uri.'/install/');
		$this->_helper->viewRenderer->setNoRender();
		$db = new Default_Model_Installsql(true);
		if(!$db->hasFirstUser()) {
			// do stuff!
			$req = $this->getRequest();
			if(is_null($req->getParam('username')))
				$this->render('databasesuccess');
			else {
				// load all the parameters
				// TODO this could be handled with Zend_Filter_Input or Zend_Form
				$this->view->username = trim($req->getParam('username'));
				$this->view->password = $req->getParam('password');
				$this->view->email = trim($req->getParam('email'));
				if(empty($this->view->username) || empty($this->view->password) || empty($this->view->email)) {
					$this->uhoh = TRUE;
					$this->render('databasesuccess');
				} elseif($db->insertFirstUser($this->view->username, $this->view->password, $this->view->email)) {
					// send user to finished screen
					$this->session->page = 5;
					$this->_redirect('http://'.$_SERVER['SERVER_NAME'].$this->view->base_uri.'/done/');
				} else {
					$this->uhoh = TRUE;
					$this->render('databasesuccess');
				}
			}
		} else {
			// user already installed
			// send user to finished screen
			$this->session->page = 5;
			$this->_redirect('http://'.$_SERVER['SERVER_NAME'].$this->view->base_uri.'/done/');
		}
	}
	/**
	 * Shows a finally done screen
	 */
	public function doneAction()
	{
		$this->view->base_uri = explode("/install", $_SERVER['REQUEST_URI']);
		$this->view->base_uri = $this->view->base_uri[0];
		$this->view->version = APPLICATION_VER;
		
		// if session page isn't 5, don't proceed
		$this->session = new Zend_Session_Namespace('installer');
		if($this->session->page < 5)
			$this->_redirect('http://'.$_SERVER['SERVER_NAME'].$this->view->base_uri.'/install/');
	}
}