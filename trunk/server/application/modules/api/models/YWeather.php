<?php
/**
 * Parses data from the Yahoo! Weather service
 *
 * @author Frederick
 * @version $Id$
 */
class Api_Model_YWeather
{
	private $_location;
	private $_conditions;
	private $_forecast;
	public function __construct ($location)
	{
		// No default location is used
		// CAXX0401 is Richmond Hill
		$this->_location = $location;
		Zend_Feed::registerNamespace('yweather', 'http://xml.weather.yahoo.com/ns/rss/1.0');
	}
	public function retrieve ()
	{
		$channel = new Zend_Feed_Rss('http://weather.yahooapis.com/forecastrss?u=c&p=' . $this->_location);
		return $channel;
	}
	public function conditions ($r)
	{
		if(!isset($this->_conditions[$r]))
		{
			$_data = $this->retrieve()->current()->{'yweather:condition'}->getDOM();
			$this->_conditions = array(
				'temp' => $_data->getAttribute('temp') ,
				'description' => $_data->getAttribute('text') ,
				'code' => $_data->getAttribute('code'));
		}
		return $this->_conditions[$r];
	}
	public function forecast ($d=0,$param)
	{
		if(!isset($this->_forecast[0]))
		{
			$_data0 = $this->retrieve()->current()->{'yweather:forecast'}[0]->getDOM();
			$_data1 = $this->retrieve()->current()->{'yweather:forecast'}[1]->getDOM();
			$this->_forecast = array(
				0 => array(
					'day' => $_data0->getAttribute('day'),
					'date' => $_data0->getAttribute('date'),
					'high' => $_data0->getAttribute('high'),
					'low' => $_data0->getAttribute('low'),
					'description' => $_data0->getAttribute('text'),
					'code' => $_data0->getAttribute('code')),
				1 => array(
					'day' => $_data1->getAttribute('day'),
					'date' => $_data1->getAttribute('date'),
					'high' => $_data1->getAttribute('high'),
					'low' => $_data1->getAttribute('low'),
					'description' => $_data1->getAttribute('text'),
					'code' => $_data1->getAttribute('code')));
		}
		return $this->_forecast[$d][$param];
	}
}