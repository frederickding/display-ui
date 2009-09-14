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
			$batch = $this->db->select()->from('dui_media', array('id' ,
				'title' ,
				'active' => 'UTC_TIMESTAMP() > activates AND (UTC_TIMESTAMP() < expires OR expires IS NULL)' ,
				'expires' => new Zend_Db_Expr('CAST(expires AS DATE)') ,
				'type' , 'weight'))->order('id DESC')->limitPage($_page, $_limit)->query()->fetchAll(Zend_Db::FETCH_ASSOC);
			$firstOne = ($_page - 1) * ($_limit) + 1;
			$lastOne = (count($batch) < $_limit) ? $firstOne + count($batch) - 1 : $firstOne + $_limit - 1;
			return array($numberOf , $batch , $firstOne , $lastOne);
		}
		return array();
	}
	/**
	 * Inserts a new multimedia item into the media table
	 * @param array $_data
	 * @param string $mime
	 * @return string
	 */
	public function insertMultimedia ($_data, $mime = 'application/octet-stream')
	{
		if (! is_null($this->db)) { // only do something if DB is connected
			// Build our dataset
			$type = in_array(pathinfo($_data['mediumfile'], PATHINFO_EXTENSION), array(
				'jpeg' , 'jpg' , 'png')) ? 'image' : (in_array(pathinfo($_data['mediumfile'], PATHINFO_EXTENSION), array(
				'mov' , 'mp4' , 'avi' , 'wmv' , 'mkv')) ? 'video' : '');
			foreach ($_data['mediumclients'] as $c) {
				$clients .= $c . ',';
			}
			// $clients is a comma-delimited list of client IDs
			$clients = substr($clients, 0, (strlen($clients) - 1));
			// now build a key => value array of all the tables needed in the insert
			$insertData = array(
				'title' => $_data['mediumtitle'] ,
				'activates' => (($_data['mediumactivatenow']) ? new Zend_Db_Expr(
					'UTC_TIMESTAMP()') : $_data['mediumactivation']) ,
				'active' => 1 ,
				'type' => $type , 'clients' => $clients ,
				'weight' => (int) $_data['mediumweight'] ,
				// FIXME: MASSIVELY BROKEN OVERWRITING WITH DUPLICATE FILENAMES
				'content' => $_data['mediumfile'] . ';' . $mime);
			if (! empty($_data['mediumexpiration'])) {
				$insertData['expires'] = $_data['mediumexpiration'];
			}
			if($type == 'image') {
				if (! empty($_data['mediumduration']) && is_numeric($_data['mediumduration'])) {
					$insertData['content'] .= ';' . $_data['mediumduration'];
				}
			}
			// insert and return the value of the auto-increment ID
			$this->db->insert('dui_media', $insertData);
			return $this->db->lastInsertId();
		}
		return '';
	}
	/**
	 * Retrieves a list of clients to which the current admin has access
	 *
	 * Note: exact clone of function from Admin_Model_Dashboard
	 * @param string|int $_admin
	 * @param int $_limit
	 * @return array
	 */
	public function fetchClients ($_admin, $_limit = 5)
	{
		if (! is_null($this->db)) {
			/*
			 * A complex SQL query to find ONLY clients to which the current user has access
			 */
			$query = $this->db->select()->from(array('c' => 'dui_clients'), array(
				'id' , 'sys_name'))->join(array('u' => 'dui_users'), 'c.admin = u.id OR c.users REGEXP CONCAT(\'(^|[0-9]*,)\', u.id, \'(,|$)\')', array())->order('c.id ASC')->limit($_limit);
			if (is_int($_admin)) {
				// treat it as the integer user ID
				$query->where('u.id = ?', $_admin, 'INTEGER');
			} else {
				// treat is as the username
				$query->where('u.username = ?', $_admin);
			}
			return $query->query()->fetchAll(Zend_Db::FETCH_ASSOC);
		}
		return array();
	}
	/**
	 * Determines whether the given administrator has permission to delete the given media file
	 * through a long-winded SQL query.
	 *
	 * If $_boolean = FALSE, this function will return the ID instead of TRUE.
	 * @param int|string $_admin
	 * @param int $_id
	 * @param bool $_boolean
	 * @return bool|int
	 */
	public function canModify ($_admin, $_id, $_boolean = TRUE)
	{
		if (! is_null($this->db)) {
			/*
			 * A complex SQL query to find ONLY media items to which the current user has access
			 * It's a lot of work because there is no admin/user column in the media table, only
			 * a clients column. We have to use SQL to find admins who have access to clients
			 * which are listed in the media item.
			 */
			$query = $this->db->select()->from(array('m' => 'dui_media'), array(
				'm.id'))->joinInner(array('c' => 'dui_clients'), 'm.clients REGEXP CONCAT( \'(^|[0-9]*,)\', c.id, \'(,|$)\' )', array())->joinInner(array(
				'u' => 'dui_users'), 'c.admin = u.id OR c.users REGEXP CONCAT( \'(^|[0-9]*,)\', u.id, \'(,|$)\' ) ', array())->where('m.id = ?', $_id)->limit(1);
			// Oops, debugging code.
			// error_log($query->assemble());
			if (is_int($_admin)) $query->where('u.id = ?', $_admin);
			else $query->where('u.username = ?', $_admin);
			$retrievedId = $this->db->fetchOne($query);
			if ($_boolean) {
				return ($_id == $retrievedId);
			} else
				return $retrievedId;
		}
		return FALSE;
	}
	/**
	 * Deletes a row from the media table
	 * @param int $_id
	 * @return bool
	 */
	public function deleteMultimedia ($_id)
	{
		$_id = (int) $_id;
		if (! is_null($this->db)) {
			$this->db->setFetchMode(Zend_Db::FETCH_OBJ);
			$query = $this->db->select()->from('dui_media', array(
				'filename' => new Zend_Db_Expr('SUBSTR(`content`, 1, LOCATE(\';\', `content`)-1)'),
				'data' => new Zend_Db_Expr($this->db->quoteIdentifier('data').' IS NULL')))->where('id = ?', $_id, 'INTEGER')->limit(1);
			$queryR = $this->db->fetchRow($query);
			$result = $this->db->delete('dui_media', $this->db->quoteInto('id = ?', $_id, 'INTEGER'));
			if ($queryR->data == 1) {
				$resultFile = unlink(realpath(MEDIA_DIR . '/' . $queryR->filename));
				$result = $result && $resultFile;
			}
			return (bool) $result;
		}
		return FALSE;
	}
}