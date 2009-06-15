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
	 * Authentication parameters verification method
	 *
	 * Checks whether provided system name and API key are valid
	 *
	 * @param string|int $sys_name the name of the client system
	 * @param string|int $api_key the associated API key
	 * @return boolean
	 * @todo need to implement a real authentication system
	 */
	public function verify ($sys_name, $api_key)
	{
		// TODO real authentication system
		if (isset($sys_name, $api_key))
			return true;
		else
			return false;
	}
}
