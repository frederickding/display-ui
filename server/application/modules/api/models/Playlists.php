<?php
/**
 * Playlists model, uses database
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
 * Provides logic for the Playlists API
 */
class Api_Model_Playlists extends Default_Model_DatabaseAbstract
{
	public function fetch($_sys_name)
	{
		$this->db->setFetchMode(Zend_Db::FETCH_OBJ);
		$query = $this->db->quote('SELECT p.`id`, p.`generated`, p.`content`
			FROM `dui_playlists` AS p 
			INNER JOIN `dui_clients` AS c ON (p.`client`=c.`id`) 
			WHERE p.`played` IS NULL 
			AND (c.`sys_name` = ? OR p.`client` IS NULL) 
			ORDER BY RAND() LIMIT 1', $_sys_name);
		$result = $this->db->fetchRow($query);
		if(is_string($result->content)) {
			return array(
				$result->id,
				$result->generated,
				unserialize($result->content)
			);
		} else return FALSE;
	}
	public function updatePlayed($_id)
	{
		$_id = (int) $_id;
		$query = array(
			'played' => new Zend_Db_Expr('UTC_TIMESTAMP()')
		);
		$this->db->update('dui_playlists', $query, 'id = '.$_id);
	}
}