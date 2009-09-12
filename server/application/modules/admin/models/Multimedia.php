<?php
/**
 * Multimedia model for control panel
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
 * Provides logic and data for admin multimedia functionality
 */
class Admin_Model_Multimedia extends Default_Model_DatabaseAbstract
{
	/**
	 * Fetches all relevant details about multimedia in the database
	 * @param int $_limit
	 * @return array
	 */
	public function fetchMultimedia ($_page = 1, $_limit = 25)
	{
		if (! is_null($this->db)) { // only do something if DB is connected
			$numberOf = $this->db->fetchOne($this->db->select()->from('dui_media', 'COUNT(*)'));
			$batch = $this->db->select()->from('dui_media', array(
				'id' ,
				'title' ,
				'active' => 'UTC_TIMESTAMP() > activates AND UTC_TIMESTAMP() < expires' ,
				'expires' => new Zend_Db_Expr('CAST(expires AS DATE)') ,
				'type' ,
				'weight'
			))->order('id DESC')->limitPage($_page, $_limit)->query()->fetchAll(Zend_Db::FETCH_ASSOC);
			$firstOne = ($_page - 1) * ($_limit) + 1;
			$lastOne = (count($batch) < $_limit) ? $firstOne + count($batch) - 1 : $firstOne + $_limit - 1;
			return array(
				$numberOf ,
				$batch ,
				$firstOne ,
				$lastOne
			);
		}
		return array();
	}
}