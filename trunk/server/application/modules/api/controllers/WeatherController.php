<?php
/**
 * Weather API
 *
 * @author Frederick
 * @version $Id$
 */
class Api_WeatherController extends Zend_Controller_Action
{
	/**
	 * The default action - show the home page
	 */
	public function indexAction ()
	{
		$authenticator = new Api_Model_Authenticator();
		$sys_name = $this->getRequest()->getParam('sys_name');
		$api_key = $this->getRequest()->getParam('api_key');
		if ($authenticator->verify($sys_name, $api_key)) {
			$server = new Zend_XmlRpc_Server();
			$server->setClass('Api_Model_YWeather');
			echo $server->handle();
		} else $this->getResponse()->setHttpResponseCode(401);
		
	}
}