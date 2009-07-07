<?php
/**
 * Authentication model for administrative interface
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
 * Provides authentication logic for admin interface
 */
class Admin_Model_Authentication
{
	/**
	 * A database configuration object
	 * @var
	 */
	private $dbconfig;
	/**
	 * An instance of the PasswordHash class
	 * @var PasswordHash
	 */
	private $PasswordHash;
	/**
	 * A connection to the database
	 * @var
	 */
	private $db;
	/**
	 * Sets up the PasswordHash class and the database
	 */
	public function __construct()
	{
		$this->PasswordHash = new Default_Model_PasswordHash(8, TRUE);
		$this->dbconfig = new Zend_Config_Ini(CONFIG_DIR.'/database.ini');
		try {
			$this->db = Zend_Db::factory($this->dbconfig->production->server->db->driver, array(
				'host'		=>	$this->dbconfig->production->server->db->hostname,
				'username'	=>	$this->dbconfig->production->server->db->username,
				'password'	=>	$this->dbconfig->production->server->db->password,
				'dbname'	=>	$this->dbconfig->production->server->db->name
			));
			$this->db->getConnection();
		} catch (Zend_Db_Adapter_Exception $e) {
			$this->db = FALSE;
			// couldn't connect
		} catch (Zend_Exception $e) {
			$this->db = FALSE;
			// couldn't load Adapter class
		}
		unset($this->dbconfig);
	}
	/**
	 * Checks whether the provided user's password is valid
	 * @param string $_user
	 * @param string $_password
	 * @return boolean|int On true, returns User ID
	 */
	public function checkPassword($_user, $_password)
	{
		$query = $this->db->quoteInto('SELECT id,password FROM dui_users WHERE username = ? LIMIT 1', $_user);
		$result = $this->db->fetchAssoc($query);
		if($this->PasswordHash->CheckPassword($_password, $result[1]['password'])) {
			return (int) $result[1]['id'];
		} else return FALSE;
	}
	/**
	 * Checks whether a given user exists
	 * @param string $_user
	 * @return boolean|int On true, returns User ID
	 */
	public function userExists($_user)
	{
		$query = $this->db->quoteInto('SELECT id FROM dui_users WHERE username = ? LIMIT 1', $_user);
		$result = $this->db->fetchOne($query);
		if(!is_empty($result)) return (int) $result;
		else return FALSE;
	}
	/**
	 * Updates a user's password
	 * @param string $_user
	 * @param string $_oldpass
	 * @param string $_newpass
	 * @return boolean
	 */
	public function changePassword($_user, $_oldpass, $_newpass)
	{
		// $result1 should have the user ID if old pass matches
		// or boolean false if old password is invalid
		$result1 = $this->checkPassword($_user, $_oldpass);
		if($result1) {
			$query = array(
				'password' => $this->PasswordHash->HashPassword($_newpass),
				'last_active' => new Zend_Db_Expr('UTC_TIMESTAMP()')
			);
			return $this->db->update('dui_users', $query, 'id = '.$result1);
		} else return FALSE;
	}
	/**
	 * Finds the ACL role of the given user
	 * @param string|int $_user
	 * @return string one of 'publisher', 'it' or 'admin', or when not found, ''
	 */
	public function userRole($_user)
	{
		if(is_int($_user)) {
			$query = $this->db->quoteInto('SELECT acl_role FROM dui_users WHERE id = ? LIMIT 1', $_user);
		} else {
			$query = $this->db->quoteInto('SELECT acl_role FROM dui_users WHERE username = ? LIMIT 1', $_user);
		}
		$result = $this->db->fetchOne($query);
		if(!is_empty($result)) return $result;
		else return '';
	}
}