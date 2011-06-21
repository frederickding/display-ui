<?php
/**
 * Options model for control panel
 *
 * Copyright 2011 Frederick Ding<br />
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
 * Provides logic and data for managing options
 */
class Admin_Model_Options extends Default_Model_DatabaseAbstract
{
	public function getOption ($_name, $_default = null, $_client = null)
	{
		$select = $this->db->select()
			->from('dui_options', 'option_value')
			->where('option_name = ?', $_name);
		if (! is_null($_client))
			$select->where('client_id = ?', $_client);
		$select->order('id DESC');
		$result = $select->query()->fetchAll(Zend_Db::FETCH_COLUMN);
		switch (count($result)) {
			case 0:
				return $_default;
			case 1:
				return $this->unserialize($result[0]);
			default:
				return $this->unserialize($result[0]);
		}
	}
	public function setOption ($_name, $_value, $_client = null, $_overwrite = true)
	{
		$_serialized = (is_string($_value)) ? $_value : serialize($_value);
		$updated = $this->db->update('dui_options',
		array(
			'option_value' => $_serialized),
		$this->db->quoteInto('option_name = ?', $_name));
		if ($updated == 1) {
			return $_value;
		} elseif ($updated == 0 && $_overwrite) {
			// create a new option
			$inserted = $this->db->insert('dui_options',
			array(
				'option_name' => $_name,
				'option_value' => $_serialized,
				'client_id' => $_client));
			return ($inserted == 1) ? $_value : null;
		}
		return null;
	}
	public function removeOption ($_name, $_client = null)
	{
		$where = $this->db->quoteInto('option_name = ?', $_name);
		if(!is_null($_client)) {
			$where .= $this->db->quoteInto(' AND client_id = ?', $_client);
		}
		$deleted = $this->db->delete('dui_options', $where);
		return ($deleted > 0) ? true : false;
	}
	public function optionExists ($_name, $_client)
	{
		$select = $this->db->select()
			->from('dui_options', new Zend_Db_Expr('COUNT(*) = 1'))
			->where('option_name = ?', $_name);
		if (! is_null($_client))
			$select->where('client_id = ?', $_client);
		$result = $select->query()->fetchColumn();
		return ($result == 1);
	}
	private function unserialize ($_value)
	{
		if (! is_string($_value))
			return $_value;
		if ($_value == 'N;')
			return null;
		if (strlen($_value) <= 3)
			return $_value;
		if ($_value == 'b:0;')
			return false;
		$unserialized = @unserialize($_value);
		if ($unserialized === false)
			// oops, this wasn't serialized; better send back the original value
			return $_value;
		return $unserialized;
	}
}