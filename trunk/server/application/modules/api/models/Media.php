<?php
/**
 * Media model, uses database
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
 * Provides logic for the Media API
 */
class Api_Model_Media extends Default_Model_DatabaseAbstract
{
	/**
	 * Determines whether a medium is stored in the database table
	 * @param $_id
	 * @return bool
	 */
	public function isStoredDb($_id)
	{
		return (bool) $this->db->fetchOne('SELECT data IS NULL AS data'
			.' FROM dui_media WHERE id = ? LIMIT 1', $_id);
	}
	/**
	 * Gets the data stored in the SQL database
	 * @param $_id
	 * @return array
	 */
	public function retrieveFromDb($_id)
	{
		$this->db->setFetchMode(Zend_Db::FETCH_ASSOC);
		$result = $this->db->select()
				->from('dui_media', array('content', 'data'))
				->where('id = ?', $_id)
				->limit(1)
				->query()
				->fetch();
		
		$content = explode(';', $result[0]['content']);
		// $mime_type = $content[1];
		
		$return = array();
		$return['filename'] = $content[0];
		$return['mime'] = $content[1];
		$return['data'] = $result[0]['data'];
		
		return $return;
	}
}