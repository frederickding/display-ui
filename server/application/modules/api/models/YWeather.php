<?php
/**
 * Parses data from the Yahoo! Weather service
 *
 */
class Api_Model_YWeather
{
	public function retrieve ($location)
	{
		$remote_feed = new Zend_Http_Client(
			'http://weather.yahooapis.com/forecastrss',
			array('maxredirects' => 0 , 'timeout' => 15)
			);
		$remote_feed->setParameterGet(array(
			'u' => 'c' ,
			'p' => $location
			));
		return $remote_feed->request()->getBody();
	}
}