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
	 * Provides a plain-text API to retrieve the current conditions
	 */
	public function indexAction ()
	{
		$this->_forward('current');
	}
	public function currentAction ()
	{
		// don't let Zend Framework render a view
		$this->_helper->viewRenderer->setNoRender();
		$YWeather = new Api_Model_YWeather($this->getRequest()->getParam('location'));
		// authenticate the request
		$Authenticator = new Api_Model_Authenticator();
		$sys_name = $this->getRequest()->getParam('sys_name');
		$api_key = $this->getRequest()->getParam('api_key');
		if ($Authenticator->verify($sys_name, $api_key)) {
			$this->getResponse()->setHeader('Content-Type', 'text/plain', true);
			echo $YWeather->conditions('temp') . "\n" . $YWeather->conditions('description') . "\n" . $YWeather->conditions('code');
		} else {
			$this->getResponse()->setHttpResponseCode(401);
			$this->view->message = 'Unauthenticated Request';
			$this->view->explanation = 'All API calls to this service must include a ' . 'valid system identifier and API key.';
			$this->render('index');
		}
	}
	public function forecastAction ()
	{
		// don't let Zend Framework render a view
		$this->_helper->viewRenderer->setNoRender();
		$YWeather = new Api_Model_YWeather($this->getRequest()->getParam('location'));
		// authenticate the request
		$Authenticator = new Api_Model_Authenticator();
		$sys_name = $this->getRequest()->getParam('sys_name');
		$api_key = $this->getRequest()->getParam('api_key');
		if ($Authenticator->verify($sys_name, $api_key)) {
			$this->getResponse()->setHeader('Content-Type', 'text/plain', true);
			switch ($this->getRequest()->getParam('day')) {
				case 1:
					echo $YWeather->forecast(1, 'high') . "\n" . $YWeather->forecast(1, 'low') . "\n" . $YWeather->forecast(1, 'description') . "\n" . $YWeather->forecast(1, 'code');
					break;
				case 0:
					echo $YWeather->forecast(0, 'high') . "\n" . $YWeather->forecast(0, 'low') . "\n" . $YWeather->forecast(0, 'description') . "\n" . $YWeather->forecast(0, 'code');
			}
		} else {
			$this->getResponse()->setHttpResponseCode(401);
			$this->view->message = 'Unauthenticated Request';
			$this->view->explanation = 'All API calls to this service must include a ' . 'valid system identifier and API key.';
			$this->render('index');
		}
	}
	public function imageAction ()
	{
		// don't let Zend Framework render a view
		$this->_helper->viewRenderer->setNoRender();
		$_code = '34';
		if($this->getRequest()->getParam('code')) {
			$_code = $this->getRequest()->getParam('code');
		} else {
			$YWeather = new Api_Model_YWeather($this->getRequest()->getParam('location'));
			$_code = $YWeather->conditions('code');
			unset($YWeather);
		}
		$image = 'http://l.yimg.com/a/i/us/nws/weather/gr/'.$_code.'d.png';
		$this->_redirect($image);
	}
}