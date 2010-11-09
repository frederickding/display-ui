<?php
/**
 * Playlists model, uses database
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
 * Provides logic for the Playlists API
 */
class Api_Model_Playlists extends Default_Model_DatabaseAbstract
{
	/**
	 * Retrieves one playlist at random from the known, stored playlists
	 * @param string $_sys_name
	 * @param int $_id
	 * @return array
	 */
	public function fetch ($_sys_name, $_id = NULL)
	{
		$this->db->setFetchMode(Zend_Db::FETCH_OBJ);
		if (is_null($_id)) {
			$query = $this->db->select()
				->from(array(
				'p' => 'dui_playlists'),
			array(
				'id',
				'generated',
				'content'))
				->joinInner(array(
				'c' => 'dui_clients'), 'p.client = c.id', '')
				->where('p.played IS NULL')
				->where('c.sys_name = ? OR p.client IS NULL', $_sys_name)
				->order('RAND()')
				->limit(1);
		} else {
			$query = $this->db->select()
				->from(array(
				'p' => 'dui_playlists'),
			array(
				'id',
				'generated',
				'content'))
				->where('p.played IS NULL')
				->where('p.id = ?', $_id)
				->limit(1);
		}
		$result = $query->query()->fetch();
		/* $query = $this->db->quoteInto('SELECT p.`id`, p.`generated`, p.`content`
			FROM `dui_playlists` AS p
			INNER JOIN `dui_clients` AS c ON (p.`client`=c.`id`)
			WHERE p.`played` IS NULL
			AND (c.`sys_name` = ? OR p.`client` IS NULL)
			ORDER BY RAND() LIMIT 1', $_sys_name);
		$result = $this->db->fetchRow($query); */
		if (is_string($result->content)) {
			return array(
				$result->id,
				$result->generated,
				$result->content);
		} else
			return FALSE;
	}
	/**
	 * Marks a playlist in the database as played
	 * by updating the 'played' column with the timestamp of retrieval
	 * @param int $_id
	 * @return int The number of rows affected
	 */
	public function updatePlayed ($_id)
	{
		$_id = (int) $_id;
		$query = array(
			'played' => new Zend_Db_Expr('UTC_TIMESTAMP()'));
		return $this->db->update('dui_playlists', $query, 'id = ' . $_id);
	}
	/**
	 * Gets media from the media table
	 * @param string $_sys_name
	 * @param int $_number
	 * @return array The results of the query
	 */
	private function getMedia ($_sys_name, $_number = 20)
	{
		$this->db->setFetchMode(Zend_Db::FETCH_ASSOC);
		$query = $this->db->select()
			->from(array(
			'm' => 'dui_media'), array(
			'id',
			'type',
			'content'))
			->joinInner(array(
			'c' => 'dui_clients'),
		'm.clients REGEXP CONCAT(\'(^|[0-9]*,)\', c.id, \'(,|$)\')', '')
				/* (^|[0-9]*,)###(,|$) searches in comma-delimited clients column
				INNER JOIN `dui_clients` AS c ON ( m.`clients`
				REGEXP CONCAT( '(^|[0-9]*,)', c.`id` , '(,|$)' ) ) */
				->where('c.sys_name = ?', $_sys_name)
			->where(
		'UTC_TIMESTAMP() > m.activates AND (UTC_TIMESTAMP() < m.expires OR m.expires IS NULL)')
			->where('m.active = 1')
			->order('m.weight DESC')
			->limit($_number);
		$result = $query->query()->fetchAll();
		return $result;
	}
	/**
	 * Builds a playlist, stores it in the database
	 * @param $_sys_name
	 * @param $_number
	 * @return int
	 */
	public function generatePlaylist ($_sys_name, $_number = 20)
	{
		$_number = (int) $_number;
		$playlist = array();
		$media = $this->getMedia($_sys_name, $_number);
		// randomize the media a little bit
		shuffle($media);
		/* array is now something like
		 * [0] => array(4) { ['id'] => 3, ['type'] => 'video', ['content'] => '...' }
		 * [1] => array(4) { ['id'] => 1, ['type'] => 'image', ['content'] => '...' }
		 */
		// build a $playlist
		foreach ($media as $medium) {
			$medium['content'] = explode(';', $medium['content']);
			/* first element is now filename / path
			 * second element is MIME type
			 * third element if it exists is duration
			 */
			// set a default duration of 20 seconds
			if (! isset($medium['content'][2]) ||
			 ! is_int($medium['content'][2]))
				$medium['content'][2] = 20;
			$playlist[] = array(
				'type' => $medium['type'],
				'filename' => $medium['content'][0],
				'duration' => ($medium['type'] == 'video') ? 'file' : $medium['content'][2],
				'media-id' => (int) $medium['id']);
		}
		$client_id = $this->db->fetchOne(
		'SELECT id FROM dui_clients WHERE sys_name = ? LIMIT 1', $_sys_name);
		$this->db->insert('dui_playlists',
		array(
			'generated' => new Zend_Db_Expr('UTC_TIMESTAMP()'),
			'client' => $client_id,
			'content' => serialize($playlist)));
		return $this->db->lastInsertId('dui_playlists');
	}
	public function buildBinary ($_playlist)
	{
		$_playlist = unserialize($_playlist);
		$playlist_objects = array();
		foreach ($_playlist as $item) {
			switch ($item['type']) {
				case 'image':
					$item['type'] = Api_Model_PlaylistItem::IMAGE_TYPE;
					break;
				case 'video':
					$item['type'] = Api_Model_PlaylistItem::VIDEO_TYPE;
					break;
				case 'text':
					$item['type'] = Api_Model_PlaylistItem::TEXT_TYPE;
					break;
				case 'ppt':
					$item['type'] = Api_Model_PlaylistItem::POWERPOINT_TYPE;
					break;
				case 'zip':
					$item['type'] = Api_Model_PlaylistItem::IMAGESHOW_TYPE;
					break;
				default:
					$item['type'] = (int) 0;
			}
			$playlist_objects[] = new Api_Model_PlaylistItem($item['media-id'],
			$item['type'], array(
				'filename' => $item['filename']), $item['duration']);
		}
		$count = 0;
		// $binary = pack('v', count($_playlist));
		$binary = '';
		foreach ($playlist_objects as $item) {
			if ($item->getType() == Api_Model_PlaylistItem::IMAGESHOW_TYPE) {
				// count zip items
				$tmp = $item->zipToString();
				$count += $tmp['count'];
				$binary .= $tmp['binary'];
			} else {
				$count ++;
				$binary .= $item->__toString();
			}
		}
		return pack('v', $count) . $binary;
	}
}