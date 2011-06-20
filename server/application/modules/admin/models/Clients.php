<?php
/**
 * Clients model for control panel
 *
 * Copyright 2009 Frederick Ding<br />
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
 * Provides logic and data for managing clients
 */
class Admin_Model_Clients extends Default_Model_DatabaseAbstract
{
	/**
	 * Retrieves an associative array of all clients to which the specified user
	 * has access, based on his/her username or user ID.
	 *
	 * @param int|string $_admin	username or user ID
	 * @return array an array of clients
	 */
	public function fetchClients ($_admin)
	{
		if (! is_null($this->db)) {
			$select = $this->db->select()
				->from(array(
				'c' => 'dui_clients'),
			array(
				'id',
				'sys_name',
				'last_active'))
				->join(array(
				'u' => 'dui_users'),
			'c.admin = u.id OR c.users REGEXP CONCAT( \'(^|[0-9]*,)\', u.id, \'(,|$)\' ) ',
			array(
				'admin' => 'username'))
				->order('id ASC');
			if (is_int($_admin)) {
				// treat it as the integer user ID
				$select->where('u.id = ?', $_admin, 'INTEGER');
			} else {
				// treat is as the username
				$select->where('u.username = ?', $_admin);
			}
			$result = $select->query()->fetchAll(Zend_Db::FETCH_ASSOC);
			// error_log($select->assemble());
			return $result;
		}
		return array();
	}
}
