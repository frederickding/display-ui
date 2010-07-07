<?php
class IndexController extends Zend_Controller_Action
{
	public function indexAction ()
	{
		// configuration object
		if(Zend_Registry::isRegistered('configuration_ini')) {
			$config = Zend_Registry::get('configuration_ini');
		} else {
			$config = new Zend_Config_ini(CONFIG_DIR . '/configuration.ini', 'production');
			Zend_Registry::set('configuration_ini', $config);
		}
		$this->view->assign('installed', ($config->server->install->live == 1));
	}
}