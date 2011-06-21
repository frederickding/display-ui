<?php
/**
 * Calendar model
 *
 * Copyright 2010 Frederick Ding<br />
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
class Api_Model_Calendar extends Default_Model_DatabaseAbstract
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
	public function getAllEvents ($client)
	{
		if (is_numeric($client)) {
			$select = $this->table
				->select(true)
				->where('client = ?', $client)
				->where('time < (NOW() + INTERVAL 7 DAY) AND time > UTC_TIMESTAMP()')
				->orWhere('type = "weekly"')
				->orWhere('TIME(time) = "00:00:00" AND DATE(time) = UTC_DATE()')
				->order('type ASC')
				->order('time ASC');
		} else {
			// treat as sysName
			$select = $this->table
				->select(true)
				->columns(array(
				'name' ,
				'time' ,
				'type' ,
				'today' => new Zend_Db_Expr('DATE(time) = CURDATE()')))
				->joinInner('dui_clients', 'dui_calendar.client = dui_clients.id', '')
				->where('time < (NOW() + INTERVAL 7 DAY) AND time > DATE(NOW())')
				->orWhere('type = "weekly"')
				->order('type ASC')
				->order('time ASC');
		}
		$events = $this->table
			->fetchAll($select);
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
	public function formatEvents (Zend_Db_Table_Rowset_Abstract $events)
	{
		$return = $events->count() . "\n";
		foreach ($events as $e) {
			if ($e->type == 'weekly') {
				// show the weekday
				$return .= date('D g:i', strtotime($e->time)) . "\n" . $e->name . "\n";
			} else if ($e->today == 1) {
				$return .= date('H:i', strtotime($e->time)) . "\n" . $e->name . "\n";
			} else
				$return .= date('M j', strtotime($e->time)) . "\n" . $e->name . "\n";
		}
		return $return;
	}
}