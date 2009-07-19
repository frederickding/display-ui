<?php 
/**
 * Headlines model, uses database
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
 * Provides logic for the Headlines API
 */
class Api_Model_Headlines {
	private $db;
	public function __construct()
	{
		$config = new Zend_Config_Ini(CONFIG_DIR.'/database.ini');
		try {
			$this->db = Zend_Db::factory($config->production->server->db->driver, array(
				'host'		=>	$config->production->server->db->hostname,
				'username'	=>	$config->production->server->db->username,
				'password'	=>	$config->production->server->db->password,
				'dbname'	=>	$config->production->server->db->name
			));
			$this->db->getConnection();
		} catch (Zend_Db_Adapter_Exception $e) {
			$this->db = FALSE;
			die('Could not connect to database');
			// couldn't connect
		} catch (Zend_Exception $e) {
			$this->db = FALSE;
			die('Could not connect to database');
			// couldn't load Adapter class
		}
	}
	public function fetch($_number = 25, $_sys_name, $_type = NULL)
	{
		$_number = (int) $_number;
		if(is_null($_type)) {
		$result = $this->db->fetchCol('SELECT h.`title` FROM `dui_headlines` AS h'
			.' INNER JOIN `dui_clients` AS c ON (h.`client`=c.`id` OR h.`client` IS NULL)'
			.' WHERE h.`active`=1 AND h.`expires`>UTC_TIMESTAMP()'
			.' AND (c.`sys_name` = \'?\' OR h.`client` IS NULL)'
			.' ORDER BY RAND() LIMIT '.$_number, $_sys_name);
		} else {
		$result = $this->db->fetchCol('SELECT h.`title` FROM `dui_headlines` AS h'
			.' INNER JOIN `dui_clients` AS c ON (h.`client`=c.`id` OR h.`client` IS NULL)'
			.' WHERE h.`active`=1 AND h.`expires`>UTC_TIMESTAMP()'
			.' AND h.`type` = '.$this->db->quote($_type)
			.' AND (c.`sys_name` = \'?\' OR h.`client` IS NULL)'
			.' ORDER BY RAND() LIMIT '.$_number, $_sys_name);
		}
		return $result;
	}
	public function fetchConcatenated($_number = 25, $_sys_name, $_type = NULL)
	{
		$headlines = $this->fetch($_number, $_sys_name, $_type);
		$result = '';
		foreach($headlines as $headline) {
			$result .= $headline . '  |  ';
		}
		$result = substr($result, 0, -5);
		return $result;
	}
}