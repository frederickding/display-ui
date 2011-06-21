<?php
/**
 * Users model for control panel
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
/**
 * Provides logic and data for managing users
 */
class Admin_Model_Users extends Default_Model_DatabaseAbstract
{
	/**
	 * Retrieves an associative array of all users.
	 *
	 * @return array an array of clients
	 */
	public function fetchUsers ()
	{
		if (! is_null($this->db)) {
			$select = $this->db->select()
				->from(array(
				'u' => 'dui_users'),
			array(
				'id',
				'username',
				'email',
				'last_active',
				'acl_role',
				'has_yubikey' => new Zend_Db_Expr('yubikey_public IS NOT NULL')))
				->order('id ASC');
			$result = $select->query()->fetchAll(Zend_Db::FETCH_ASSOC);
			// error_log($select->assemble());
			return $result;
		}
		return array();
	}
	public function insertUser ($_username, $_password, $_email, $_acl_role,
	$_yubikey)
	{
		if(is_null($this->db))
			return false;
		$PasswordHash = new Default_Model_PasswordHash(8, false);
		$data = array(
			'username' => $_username,
			'password' => $PasswordHash->HashPassword($_password),
			'email' => $_email,
			'last_active' => new Zend_Db_Expr('UTC_TIMESTAMP()'),
			'acl_role' => $_acl_role
		);
		if(isset($_yubikey) && strlen($_yubikey) >= 12) {
			$data['yubikey_public'] = substr($_yubikey, 0, 12);
		}
		$insert = $this->db->insert('dui_users', $data);
		return $insert;
	}
}