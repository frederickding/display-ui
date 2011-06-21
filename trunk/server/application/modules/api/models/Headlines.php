<?php
/**
 * Headlines model, uses database
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
 * Provides logic for the Headlines API
 */
class Api_Model_Headlines extends Default_Model_DatabaseAbstract
{
	public function whatDayIsIt ()
	{
		// system installation date is always Day 1
		if (Zend_Registry::isRegistered('configuration_ini')) {
			$this->config = Zend_Registry::get('configuration_ini');
		} else {
			$this->config = new Zend_Config_Ini(
			CONFIG_DIR . '/configuration.ini', 'production');
			Zend_Registry::set('configuration_ini', $this->config);
		}
		// now find the number of days between now and then, using MySQL
		$installDate = $this->config->server->install->date;
		$totalDateDiff = $this->db->select()
			->from('duix_alternating_exceptions',
		array(
			new Zend_Db_Expr('DATEDIFF(UTC_DATE(), ?) - COUNT(*)')))
			->where('date < UTC_DATE() AND date > ?')
			->query(null, array(
			$installDate,
			$installDate))
			->fetchColumn();
		return $totalDateDiff % 2 + 1;
	}
	/**
	 * Retrieves the headlines from the database, returns as an array
	 * @param int $_number [optional]
	 * @param string $_sys_name
	 * @param string $_type [optional]
	 * @return array
	 */
	public function fetch ($_number = 25, $_sys_name, $_type = NULL)
	{
		$_number = (is_null($_number)) ? 25 : (int) $_number;
		$query = $this->db
			->select()
			->from(array(
			'h' => 'dui_headlines'), 'title')
			->joinInner(array(
			'c' => 'dui_clients'), 'h.client = c.id OR h.client IS NULL', array())
			->where('h.active = 1')
			->where('h.expires > UTC_TIMESTAMP()')
			->where('c.sys_name = ? OR h.client IS NULL', $_sys_name)
			->order('RAND()')
			->limit($_number);
		if (! is_null($_type)) {
			$query = $query->where('h.type = ?', $_type);
		}
		return $query->query()
			->fetchAll(Zend_Db::FETCH_COLUMN);
	}
	/**
	 * Retrieves the headlines from the database, creates a | delimited string
	 * @param int $_number [optional]
	 * @param string $_sys_name
	 * @param string $_type [optional]
	 * @return string
	 */
	public function fetchConcatenated ($_number = 25, $_sys_name, $_type = NULL, $_day = null)
	{
		$_number = (is_null($_number)) ? 25 : (int) $_number;
		$query = $this->db
			->select()
			->from(array(
			'h' => 'dui_headlines'), 'title')
			->joinInner(array(
			'c' => 'dui_clients'), 'h.client = c.id OR h.client IS NULL', array())
			->where('h.active = 1')
			->where('h.expires > UTC_TIMESTAMP()')
			->where('c.sys_name = ? OR h.client IS NULL', $_sys_name)
			->order('RAND()')
			->limit($_number);
		if (! is_null($_type)) {
			$query->where('h.type = ?', $_type);
		}
		if($_day == 1 || $_day == 2) {
			$query->where('h.alternating = ? OR h.alternating = 0 OR h.alternating IS NULL', $_day);
		}
		$headlines = $query->query()
			->fetchAll(Zend_Db::FETCH_COLUMN);
		$result = '';
		foreach ($headlines as $headline) {
			$result .= $headline . '  |  ';
		}
		$result = substr($result, 0, - 5);
		return $result;
	}
}