<?php
/**
 * Headlines model for control panel
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
 * Provides logic and data for managing headlines
 */
class Admin_Model_Headlines extends Default_Model_DatabaseAbstract
{
	public function fetchHeadlines ($_admin)
	{
		if (! is_null($this->db)) {
			// Fetch headlines for clients to which the active user has access
			$select = $this->db->select()->from(array(
				'h' => 'dui_headlines'), array(
				'id' ,
				'title' ,
				'active' ,
				'expires' => new Zend_Db_Expr('CAST(expires AS DATE)') ,
				'type'))
			->join(array('c' => 'dui_clients'), 'h.client = c.id', array('sys_name'))
			->join(array('u' => 'dui_users'), 'c.admin = u.id OR c.users REGEXP CONCAT( \'(^|[0-9]*,)\', u.id, \'(,|$)\' ) ', array())
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
	/**
	 * Determines whether the given administrator has permission to modify the given headline
	 *
	 * If $_boolean = FALSE, this function will return the ID instead of TRUE.
	 * @param int|string $_admin
	 * @param int $_id
	 * @param bool $_boolean
	 * @return bool|int
	 */
	public function canModify ($_admin, $_id, $_boolean = TRUE)
	{
		if (! is_null($this->db)) {
			/*
			 * A complex SQL query to find ONLY headlines to which the current user has access
			 * It's a lot of work because there is no admin/user column in the headlines table, only
			 * a client column. We have to use SQL to find admins who have access to clients
			 * which are listed in the row.
			 */
			$query = $this->db->select()->from(array(
				'h' => 'dui_headlines'), array(
				'h.id'))->joinInner(array(
				'c' => 'dui_clients'), 'h.client = c.id', array())->joinInner(array(
				'u' => 'dui_users'), 'c.admin = u.id OR c.users REGEXP CONCAT( \'(^|[0-9]*,)\', u.id, \'(,|$)\' ) ', array())->where('h.id = ?', $_id)->limit(1);
			// Oops, debugging code.
			// error_log($query->assemble());
			if (is_int($_admin)) $query->where('u.id = ?', $_admin);
			else $query->where('u.username = ?', $_admin);
			$retrievedId = $this->db->fetchOne($query);
			if ($_boolean) {
				return ($_id == $retrievedId);
			} else
				return $retrievedId;
		}
		return FALSE;
	}
	/**
	 * Deletes a row from the headlines table
	 * @param int $_id
	 * @return bool
	 */
	public function deleteHeadline ($_id)
	{
		$_id = (int) $_id;
		if (! is_null($this->db)) {
			$result = $this->db->delete('dui_headlines', $this->db->quoteInto('id = ?', $_id, 'INTEGER'));
			return (bool) $result;
		}
		return FALSE;
	}
}
