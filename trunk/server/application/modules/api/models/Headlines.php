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
class Api_Model_Headlines extends Default_Model_DatabaseAbstract
{
	public function fetch($_number = 25, $_sys_name, $_type = NULL)
	{
		$_number = (is_null($_number)) ? 25 : (int) $_number;
		if(is_null($_type)) {
		$result = $this->db->fetchCol('SELECT h.`title` FROM `dui_headlines` AS h'
			.' INNER JOIN `dui_clients` AS c ON (h.`client`=c.`id` OR h.`client` IS NULL)'
			.' WHERE h.`active`=1 AND h.`expires`>UTC_TIMESTAMP()'
			.' AND (c.`sys_name` = '.$this->db->quote($_sys_name).' OR h.`client` IS NULL)'
			.' ORDER BY RAND() LIMIT '.$_number);
		} else {
		$result = $this->db->fetchCol('SELECT h.`title` FROM `dui_headlines` AS h'
			.' INNER JOIN `dui_clients` AS c ON (h.`client`=c.`id` OR h.`client` IS NULL)'
			.' WHERE h.`active`=1 AND h.`expires`>UTC_TIMESTAMP()'
			.' AND h.`type` = '.$this->db->quote($_type)
			.' AND (c.`sys_name` = '.$this->db->quote($_sys_name).' OR h.`client` IS NULL)'
			.' ORDER BY RAND() LIMIT '.$_number);
		}
		return $result;
	}
	public function fetchConcatenated($_number = 25, $_sys_name, $_type = NULL)
	{
		$_number = (is_null($_number)) ? 25 : (int) $_number;
		if(is_null($_type)) {
		$headlines = $this->db->fetchCol('SELECT h.`title` FROM `dui_headlines` AS h'
			.' INNER JOIN `dui_clients` AS c ON (h.`client`=c.`id` OR h.`client` IS NULL)'
			.' WHERE h.`active`=1 AND h.`expires`>UTC_TIMESTAMP()'
			.' AND (c.`sys_name` = '.$this->db->quote($_sys_name).' OR h.`client` IS NULL)'
			.' ORDER BY RAND() LIMIT '.$_number);
		} else {
		$headlines = $this->db->fetchCol('SELECT h.`title` FROM `dui_headlines` AS h'
			.' INNER JOIN `dui_clients` AS c ON (h.`client`=c.`id` OR h.`client` IS NULL)'
			.' WHERE h.`active`=1 AND h.`expires`>UTC_TIMESTAMP()'
			.' AND h.`type` = '.$this->db->quote($_type)
			.' AND (c.`sys_name` = '.$this->db->quote($_sys_name).' OR h.`client` IS NULL)'
			.' ORDER BY RAND() LIMIT '.$_number);
		}
		$result = '';
		foreach($headlines as $headline) {
			$result .= $headline . '  |  ';
		}
		$result = substr($result, 0, -5);
		return $result;
	}
}