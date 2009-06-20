<?php
/**
 * Authenticator model
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
 * Authenticates API requests
 */
class Api_Model_Authenticator
{
	/**
	 * Stores the unix timestamp of 1 year in the future, rounded
	 *
	 * @var string
	 */
	private $_install_date = '';
	/**
	 * Holds the configuration read from Zend_Config
	 *
	 * @var Zend_Config_Ini
	 */
	private $_config;
	/**
	 * Constructor method
	 *
	 * Loads the configuration, calculates the unix timestamp using install date
	 */
	public function __construct ()
	{
		$this->_config = new Zend_Config_Ini(CONFIG_DIR . '/configuration.ini',
											'production');
		$this->_install_date = round(strtotime('+1 year',
			strtotime($this->_config->production->server->install->date)), - 3);
	}
	/**
	 * Authentication parameters verification method
	 *
	 * Checks whether provided system name and API key are valid
	 *
	 * @param string|int $sys_name the name of the client system
	 * @param string|int $api_key the associated API key
	 * @return boolean
	 */
	public function verify ($sys_name = '', $api_key = '')
	{
		// Make sure required parameters are provided
		if (! $sys_name || ! $api_key)
			return false;
		// Check whether a valid API key matches the provided key
		elseif ($this->generate($sys_name) == $api_key)
			return true;
		else
			return false;
	}
	/**
	 * API key generator method
	 *
	 * Generates a valid API key for the given client system name
	 *
	 * @param string $sys_name the name of the client system
	 * @return false|string
	 */
	public function generate ($sys_name = '')
	{
		// Make sure required parameter is provided
		if (! $sys_name)
			return false;
		// Generate a valid API key using SHA1.
		// HMAC with the SHA256 algorithm generates a message digest
		// for the system install date, using the client system name as the secret
		$valid_api_key = sha1(hash_hmac('sha256', $this->_install_date, $sys_name)
			.$this->_config->production->server->install->secret);
		return $valid_api_key;
	}
}
