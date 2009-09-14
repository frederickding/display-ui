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
			$select = $this->db->select()->from(array('h' => 'dui_headlines'), array('id' ,
				'title' , 'active', 'expires' => new Zend_Db_Expr('CAST(expires AS DATE)'), 'type'))
			->joinInner(array('c' => 'dui_clients'), 'h.client = c.id', array('sys_name'))
			->joinInner(array('u' => 'dui_users'), 'c.admin = u.id OR c.users REGEXP CONCAT( \'(^|[0-9]*,)\', u.id, \'(,|$)\' ) ', array())
			// combine ones to which the user has access with ones to which all users have access
			->union(
				$this->db->select()
				->from('dui_headlines', array('id', 'title', 'active', 'expires' => new Zend_Db_Expr('CAST(expires AS DATE)'), 'type'))
				->where('('.$this->db->quoteIdentifier('client').' IS NULL)'))
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
