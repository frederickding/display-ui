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
class Api_Model_Authenticator {
    /**
     * Stores today's date in YYYY-MM-DD format, GMT
     *
     * @var string
     */
    private $_date = '';
    /**
     * Holds the configuration read from Zend_Config
     *
     * @var Zend_Config_Ini
     */
    private $_config;
    /**
     * Constructor method
     *
     * Loads the configuration, gets the current GMT date
     */
    public function __construct() {
        $this->_config = new Zend_Config_Ini(CONFIG_DIR.'/configuration.ini', 'production');
        $this->_date = gmdate('Y-m-d');
    }
    /**
     * Authentication parameters verification method
     *
     * Checks whether provided system name and signature are valid
     *
     * @param string|int $sys_name the name of the client system
     * @param string $signature the signature of the request
     * @return boolean
     */
    public function verify($sys_name = '', $signature = '') {
        // Make sure required parameters are provided
        if (!$sys_name || !$signature || strlen($signature) != 40)
            return false;
        // Check whether a valid API key matches the provided key
        elseif (sha1($this->apiKey($sys_name).$this->_date) == $signature)
            return true;
        else
            return false;
    }
	/**
	 * API key generator method
	 * 
	 * Generates a valid internal API key for the given client system name
	 * @param string $sys_name the name of the client system
	 * @return false|string
	 */
	public function apiKey($sys_name = '') {
		// Make sure required parameter is provided
		if (!$sys_name)
			return false;
		return hash_hmac('sha256', $sys_name, $this->_config->production->server->install->secret);
	}
}
