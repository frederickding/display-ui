<?php
/**
 * Media model, uses database
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
 * Provides logic for the Media API
 */
class Api_Model_Media extends Default_Model_DatabaseAbstract
{
	/**
	 * Determines whether a medium is stored in the database table
	 * @param $_id
	 * @return int
	 */
	public function isStoredDb ($_id)
	{
		$result = $this->db
			->fetchOne('SELECT data IS NOT NULL AS data' . ' FROM dui_media WHERE id = ? LIMIT 1', $_id);
		if ($result == 1) return 1;
		elseif ($result == 0) return 0;
		else return - 1;
	}
	/**
	 * Gets the data stored in the SQL database
	 * @param $_id
	 * @return array
	 */
	public function retrieveFromDb ($_id)
	{
		$this->db
			->setFetchMode(Zend_Db::FETCH_ASSOC);
		$result = $this->db
			->select()
			->from('dui_media', array(
			'content' ,
			'data' ,
			'data_size' => 'OCTET_LENGTH(data)'))
			->where('id = ?', $_id)
			->limit(1)
			->query()
			->fetch();
		$content = explode(';', $result['content']);
		return array(
			'filename' => $content[0] ,
			'mime' => $content[1] ,
			'data' => $result['data'] ,
			'filesize' => $result['data_size']);
	}
	/**
	 * Gets the data about a file stored in the filesystem
	 * @param $_id
	 * @return array|bool Returns false on file non-existent
	 */
	public function retrieveFromFile ($_id)
	{
		$this->db
			->setFetchMode(Zend_Db::FETCH_ASSOC);
		$result = $this->db
			->select()
			->from('dui_media', array(
			'content'))
			->where('id = ?', $_id)
			->limit(1)
			->query()
			->fetch();
		if (empty($result['content'])) return FALSE;
		$content = explode(';', $result['content']);
		$return = array(
			'filename' => MEDIA_DIR . '/' . $content[0] ,
			'mime' => $content[1] ,
			'filesize' => filesize(MEDIA_DIR . '/' . $content[0]));
		if (! file_exists($return['filename']) || $return['filesize'] == 0) return FALSE;
		return $return;
	}
}