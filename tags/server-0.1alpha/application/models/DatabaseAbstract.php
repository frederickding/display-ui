<?php
/**
 * Abstract Database class extended by models requiring the database
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
 * Provides logic and database functionality
 */
abstract class Default_Model_DatabaseAbstract
{
	/**
	 * Stores an instance of the database adapter
	 * @var Zend_Db_Adapter_Abstract
	 */
	protected $db = NULL;
	/**
	 * Connects to the database
	 * This behaviour can be overridden by a child class constructor
	 */
	public function __construct ()
	{
		// connect to the database to test
		$this->connectDatabase();
	}
	/**
	 * Initiates the database adapter and tries to connect to the database
	 */
	public function connectDatabase()
	{
		if (is_null($this->db)) {
			$config = new Zend_Config_Ini(CONFIG_DIR . '/database.ini');
			try {
				$this->db = Zend_Db::factory($config->production->server->db->driver, array(
					'host' => $config->production->server->db->hostname ,
					'username' => $config->production->server->db->username ,
					'password' => $config->production->server->db->password ,
					'dbname' => $config->production->server->db->name ,
					'charset' => 'utf8'
				));
				$this->db->getConnection();
				unset($config);
			} catch (Zend_Db_Adapter_Exception $e) {
				$this->db = NULL;
				die('Could not connect to database');
				// couldn't connect
			} catch (Zend_Exception $e) {
				$this->db = NULL;
				die('Could not connect to database');
				// couldn't load Adapter class
			}
		}
	}
	/**
	 * Tests whether the database is connected, and disconnects it
	 * @return bool
	 */
	public function disconnectDatabase ()
	{
		if (!is_null($this->db)) {
			if ($this->db->isConnected() && $this->db->closeConnection()) return TRUE;
		}
		return FALSE;
	}
	/**
	 * Upon destructing this object, disconnect the database
	 */
	public function __destruct ()
	{
		$this->disconnectDatabase();
	}
}