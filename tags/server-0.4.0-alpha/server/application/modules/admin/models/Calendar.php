<?php
/**
 * Calendar model for administrative interface
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
 * Calendar-related functionality
 */
class Admin_Model_Calendar extends Default_Model_DatabaseAbstract
{
	/**
	 * A Zend_Db_Table object for the calendar table; makes accessing it much
	 * easier.
	 * @var Zend_Db_Table_Abstract
	 */
	protected $table = null;
	public function __construct ()
	{
		$this->connectDatabase();
		$this->table = new Zend_Db_Table(
			array(
				'db' => $this->db ,
				'name' => 'dui_calendar'));
	}
	public function getAllEvents ()
	{
		$select = $this->db
			->select()
			->from('dui_calendar', '*')
			->joinInner('dui_clients', 'dui_calendar.client = dui_clients.id', 'dui_clients.sys_name')
			->order('type ASC')
			->order('time ASC');
		$events = $select->query()
			->fetchAll(Zend_Db::FETCH_OBJ);
		return $events;
	}
	public function insertEvent ($name, $time, $client, $visible = true, $type = 'once')
	{
		$client = (int) $client;
		$visible = (bool) $visible;
		if ($type != 'once' && $type != 'weekly') {
			$type = 'once';
		}
		return $this->table
			->insert(array(
			'name' => $name ,
			'time' => $time ,
			'visible' => $visible ,
			'type' => $type ,
			'client' => $client));
	}
	public function deleteEvent ($id)
	{
		$_id = (int) $id;
		if (! is_null($this->db)) {
			$result = $this->table
				->delete($this->db
				->quoteInto('event_id = ?', $_id, 'INTEGER'));
			return (bool) $result;
		}
		return false;
	}
}