<?php
abstract class Admin_ControllerAbstract extends Zend_Controller_Action
{
	/**
	 * @var Zend_Session_Namespace
	 */
	protected $auth_session;
	/**
	 * @var Zend_Config
	 */
	protected $config;
	protected $Acl;
	protected $reqController = '';
	protected $reqAction = '';
	public function init ()
	{
		// initiate a session for the installer
		$this->auth_session = new Zend_Session_Namespace('auth');
		// ALWAYS check if authenticated
		if (! $this->auth_session->authenticated) {
			return $this->_redirect(
			$this->view->serverUrl() . $this->view->url(
			array(
				'module' => 'admin',
				'controller' => 'login',
				'action' => 'index')) . '?redirect=' . $this->view->url(
			array(
				'module' => 'admin',
				'controller' => $this->_request->getControllerName(),
				'action' => $this->_request->getActionName())));
		}
		$this->reqController = $this->_request->getControllerName();
		$this->reqAction = $this->_request->getActionName();
		$this->_helper->layout()->setLayout('AdminPanelWidgets');
		$this->view->username = $this->auth_session->username;
		// deal with ACL restrictions
		$this->Acl = new Admin_Model_Acl();
		if (! $this->Acl->isAllowed($this->auth_session->userRole,
		$this->reqController, $this->reqAction)) {
			$this->view->assign('userRole', $this->auth_session->userRole);
			$this->view->assign('resource',
			$this->reqController . '/' . $this->reqAction);
			return $this->render('not-allowed', null, true);
		}
		// otherwise proceed with load
		// configuration object
		if (Zend_Registry::isRegistered('configuration_ini')) {
			$this->config = Zend_Registry::get('configuration_ini');
		} else {
			$this->config = new Zend_Config_Ini(
			CONFIG_DIR . '/configuration.ini', 'production');
			Zend_Registry::set('configuration_ini', $this->config);
		}
		$this->view->systemName = $this->config->server->install->name;
		return true;
	}
}