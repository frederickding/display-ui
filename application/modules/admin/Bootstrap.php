<?php
class Admin_Bootstrap extends Zend_Application_Module_Bootstrap
{
	protected function _initRoutes()
	{
		$router = Zend_Controller_Front::getInstance()->getRouter();
		// a better logout URL /admin/logout/ instead of the nonsensical /admin/login/logout/
		$route[0] = new Zend_Controller_Router_Route_Static('admin/logout/',
					array('module' => 'admin', 'controller' => 'login', 'action' => 'logout'));
		$router->addRoute('login', $route[0]);
	}
}