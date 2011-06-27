<?php
require 'ControllerAbstract.php';
class Admin_OptionsController extends Admin_ControllerAbstract
{
	public function indexAction ()
	{
		if (! is_null($this->config))
			$this->view->appConfig = $this->config->toArray();
	}
}