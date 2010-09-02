<?php
/**
 * Dashboard model for control panel
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
 * Provides logic and data for admin dashboard
 */
class Admin_Model_Dashboard extends Default_Model_DatabaseAbstract
{
	/**
	 * Retrieves a number of statistics about the current system
	 * @return array
	 */
	public function fetchStatusReport ()
	{
		if (! is_null($this->db)) { // only do something if DB is connected
			$query = $this->db
				->select()
				->from('information_schema.TABLES', array(
				'TABLE_NAME' ,
				'TABLE_ROWS'))
				->where('TABLE_NAME LIKE \'dui_%\'');
			$result = $this->db
				->fetchPairs($query);
			$result2 = $this->db
				->fetchOne($this->db
				->select()
				->from('dui_media', 'COUNT(*)')
				->where('type = ?', Admin_Model_Multimedia::IMAGE_TYPE));
			$result3 = $this->db
				->fetchOne($this->db
				->select()
				->from('dui_media', 'COUNT(*)')
				->where('type = ?', Admin_Model_Multimedia::VIDEO_TYPE));
			$result4 = $this->db
				->fetchOne($this->db
				->select()
				->from('dui_media', 'COUNT(*)')
				->where('type = ?', Admin_Model_Multimedia::POWERPOINT_TYPE));
			return array(
				'clients' => $result['dui_clients'] ,
				'headlines' => $result['dui_headlines'] ,
				'users' => $result['dui_users'] ,
				'images' => $result2 ,
				'videos' => $result3 ,
				'powerpoints' => $result4);
		}
		return array();
	}
	/**
	 * Retrieves a list of clients active within the 12 hours
	 * @param int $_limit
	 * @return array
	 */
	public function fetchActiveClients ($_limit = 5)
	{
		if (! is_null($this->db)) {
			/* select only clients that submitted API queries within the past 12 hours,
			 * and order them by last_active time
			 */
			$query = $this->db
				->select()
				->from('dui_clients', array(
				'id' ,
				'sys_name' ,
				'last_active' ,
				'time_diff' => new Zend_Db_Expr(
					'TIMESTAMPDIFF (MINUTE, ' . $this->db
						->quoteIdentifier('last_active') . ', UTC_TIMESTAMP() )')))
				->where('TIMESTAMPDIFF (HOUR, ' . $this->db
				->quoteIdentifier('last_active') . ', UTC_TIMESTAMP() ) < 12')
				->order('last_active DESC')
				->limit($_limit)
				->query();
			// fetch the query
			$resultset = $query->fetchAll(Zend_Db::FETCH_ASSOC);
			return $resultset;
		}
		return array();
	}
	/**
	 * Retrieves a list of clients to which the current admin has access
	 * @param string|int $_admin
	 * @param int $_limit
	 * @return array
	 */
	public function fetchClients ($_admin, $_limit = 5)
	{
		if (! is_null($this->db)) {
			/*
			 * A complex SQL query to find ONLY clients to which the current user has access
			 */
			$query = $this->db
				->select()
				->from(array(
				'c' => 'dui_clients'), array(
				'id' ,
				'sys_name'))
				->join(array(
				'u' => 'dui_users'), 'c.admin = u.id OR c.users REGEXP CONCAT(\'(^|[0-9]*,)\', u.id, \'(,|$)\')', array())
				->order('c.id ASC')
				->limit($_limit);
			if (is_int($_admin)) {
				// treat it as the integer user ID
				$query->where('u.id = ?', $_admin);
			} else {
				// treat is as the username
				$query->where('u.username = ?', $_admin);
			}
			return $query->query()
				->fetchAll(Zend_Db::FETCH_ASSOC);
		}
		return array();
	}
	/**
	 * Inserts a headline into the database from the Quickline form
	 * @param string $_title
	 * @param int $_clientId
	 * @return bool
	 */
	public function insertQuickline ($_title, $_clientId)
	{
		if (! is_null($this->db)) {
			$this->db
				->insert('dui_headlines', array(
				'title' => trim($_title) ,
				'active' => 1 ,  // make it expire in 1 month by default
				'expires' => new Zend_Db_Expr(
					'DATE_ADD(UTC_TIMESTAMP(), INTERVAL 1 MONTH)') ,
				'type' => 'news' ,
				'client' => $_clientId));
			if ($this->db
				->lastInsertId()) return TRUE;
		}
		return FALSE;
	}
}