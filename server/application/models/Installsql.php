<?php
/**
 * Database model for installer
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
 * Provides database access for the installer
 */
class Default_Model_Installsql
{
	private $_driver;
	private $_host;
	private $_username;
	private $_password;
	private $_dbname;
	private $db;
	/**
	 * Initializes private class variables
	 * @param string $_host
	 * @param string $_username
	 * @param string $_password
	 * @param string $_dbname
	 * @param string $_driver [optional]
	 */
	public function __construct($_host='localhost', $_username='', $_password='', $_dbname='', $_driver='Pdo_Mysql')
	{
		$this->_host = trim($_host);
		$this->_username = trim($_username);
		$this->_password = $_password;
		$this->_dbname = trim($_dbname);
		$this->_driver = (in_array($_driver, array('Pdo_Mysql', 'Mysqli'))) ? $_driver : 'Pdo_Mysql';
		try {
			$this->db = Zend_Db::factory($this->_driver, array(
				'host'		=>	$this->_host,
				'username'	=>	$this->_username,
				'password'	=>	$this->_password,
				'dbname'	=>	$this->_dbname
			));
			$this->db->getConnection();
		} catch (Zend_Db_Adapter_Exception $e) {
			$this->db = FALSE;
			// couldn't connect
		} catch (Zend_Exception $e) {
			$this->db = FALSE;
			// couldn't load Adapter class
		}
	}
	/**
	 * Tests whether the connection was successful
	 * @return boolean
	 */
	public function test()
	{
		if($this->db == FALSE) return FALSE;
		else return TRUE;
	}
	
	public function installStructure()
	{
		$query = file_get_contents(APPLICATION_PATH.'/controllers/structure.mysql.sql')
			or die('Couldn\'t load SQL query.');
		if($this->_driver == 'Mysqli') {
			// operate in MySQLi mode
			$result = $this->db->getConnection()->query($query, MYSQLI_USE_RESULT);
		} elseif($this->_driver == 'Pdo_Mysql') {
			// operate in PDO mode
			$result = $this->db->getConnection()->exec($query);
		}
		if($result !== FALSE) return TRUE;
		else return FALSE;
	}
}