<?php
/**
 * Parses data from the Yahoo! Weather service
 *
 */
class Api_Model_YWeather
{
	private $_location;
	private $_conditions;
	public function __construct ($location)
	{
		$this->_location = $location;
		Zend_Feed::registerNamespace('yweather', 'http://xml.weather.yahoo.com/ns/rss/1.0');
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
	public function conditions ($r)
	{
		if(!isset($this->_conditions['code']))
		{
			$_data = $this->retrieve()->current()->{'yweather:condition'}->getDOM();
			$this->_conditions = array(
				'temp' => $_data->getAttribute('temp') ,
				'description' => $_data->getAttribute('text') ,
				'code' => $_data->getAttribute('code'));
		}
		return $this->_conditions[$r];
	}
}