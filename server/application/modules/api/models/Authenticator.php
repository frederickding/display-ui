<?php
/**
 * Authenticates API requests
 *
 * @author Frederick
 * @version $Id$
 */
class Api_Model_Authenticator
{
	public function verify ($sys_name, $api_key)
	{
		// TODO real authentication system
		if (isset($sys_name, $api_key))
			return true;
		else
			return false;
	}
}
