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
		$this->_helper->viewRenderer->setNoRender();
		$Authenticator = new Api_Model_Authenticator();
		$YWeather = new Api_Model_YWeather();
		$sys_name = $this->getRequest()->getParam('sys_name');
		$api_key = $this->getRequest()->getParam('api_key');
		if ($Authenticator->verify($sys_name, $api_key)) {
			echo $YWeather->retrieve($this->getRequest()->getParam('location'));
		} else $this->getResponse()->setHttpResponseCode(401);
		
	}
}