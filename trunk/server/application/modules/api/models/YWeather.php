<?php
/**
 * Parses data from the Yahoo! Weather service
 *
 */
class Api_Model_YWeather
{
	private $_location;
	public function __construct ($location)
	{
		$this->_location = $location;
	}
	public function retrieve ()
	{
		$channel = new Zend_Feed_Rss('http://weather.yahooapis.com/forecastrss?u=c&p=' . $this->_location);
		return $channel;
		/* $remote_feed = new Zend_Http_Client(
			'http://weather.yahooapis.com/forecastrss',
			array('maxredirects' => 0 , 'timeout' => 15)
			);
		$remote_feed->setParameterGet(array(
			'u' => 'c' ,
			'p' => $location
			));
		return $remote_feed->request()->getBody(); */
	}
	public function temperature ()
	{
		$_data = $this->retrieve();
		var_dump($_data);
	}
}