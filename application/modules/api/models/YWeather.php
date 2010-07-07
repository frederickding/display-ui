<?php
/**
 * Weather API model
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
 * Parses data from the Yahoo! Weather service
 */
class Api_Model_YWeather
{
	private $_location;
	private $_city = '';
	private $_conditions;
	private $_forecast;
	/**
	 * Constructor
	 *
	 * Initializes the $_location variable, which is used to
	 * retrieve location-based weather data.
	 * @param string $location current location as ZIP code or ID
	 */
	public function __construct ($location)
	{
		// Default location is used:
		// CAXX0504 (Toronto, ON, Canada)
		$this->_location = (!empty($location)) ? $location : 'CAXX0504';
		Zend_Feed::registerNamespace('yweather', 'http://xml.weather.yahoo.com/ns/rss/1.0');
	}
	/**
	 * RSS retriever
	 *
	 * Retrieves the RSS feed containing weather data
	 * @return Zend_Feed_Rss
	 */
	public function retrieve ()
	{
		$channel = new Zend_Feed_Rss('http://weather.yahooapis.com/forecastrss?u=c&p='
									. $this->_location);
		return $channel;
	}
	public function getCity ()
	{
		if($this->_city == '')
		{
			$this->_city = $this->retrieve()->{'yweather:location'}->getDOM()->getAttribute('city');
		}
		return $this->_city;
	}
	/**
	 * Current conditions processor
	 *
	 * Returns current conditions
	 * @param string $r desired data
	 * @return string
	 */
	public function conditions ($r)
	{
		if(!isset($this->_conditions[$r]))
		{
			$_data = $this->retrieve()->current()->{'yweather:condition'}->getDOM();
			$this->_conditions = array(
				'temp' => (int) $_data->getAttribute('temp') ,
				'description' => $_data->getAttribute('text') ,
				'code' => (int) $_data->getAttribute('code'));
		}
		return $this->_conditions[$r];
	}
	/**
	 * Forecast method
	 *
	 * Determines parameters for 2-day forecast
	 * @param integer $d 0 for the following day, 1 for the day after
	 * @param string $param
	 * @return string
	 */
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