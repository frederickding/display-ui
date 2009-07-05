<?php
class Admin_Bootstrap extends Zend_Application_Module_Bootstrap
{
	protected function _initRoutes()
	{
		$router = Zend_Controller_Front::getInstance()->getRouter();
		$route[0] = new Zend_Controller_Router_Route_Static('admin/login/',
					array('module' => 'admin', 'controller' => 'index', 'action' => 'login'));
		$router->addRoute('login', $route[0]);
	}
}