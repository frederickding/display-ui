<?php
/**
 * IndexController
 *
 * @author Frederick
 * @version $Id$
 */
// require_once 'Zend/Controller/Action.php';
class Link_IndexController extends Zend_Controller_Action
{
	public function init ()
	{
		// initiate a session for authorization
		$this->auth_session = new Zend_Session_Namespace('auth');
		// ALWAYS check if authenticated
		if (! $this->auth_session->authenticated) {
			return $this->_redirect($this->view->serverUrl() . $this->view->url(array(
				'module' => 'admin' ,
				'controller' => 'login' ,
				'action' => 'index')) . '?redirect=' . $this->view->url(array(
				'module' => 'link' ,
				'controller' => $this->_request->getControllerName() ,
				'action' => $this->_request->getActionName())));
		}
		// configuration object
		if (Zend_Registry::isRegistered('configuration_ini')) {
			$config = Zend_Registry::get('configuration_ini');
		} else {
			$config = new Zend_Config_ini(CONFIG_DIR . '/configuration.ini',
				'production');
			Zend_Registry::set('configuration_ini', $config);
		}
		// ACL
		if (! Zend_Registry::isRegistered('dui_acl')) {
			$this->acl = new Admin_Model_Acl();
			Zend_Registry::set('dui_acl', $this->acl);
		} else {
			$this->acl = Zend_Registry::get('dui_acl');
		}
		$this->auth = new Admin_Model_Authentication();
		$this->user_role = $this->auth->userRole($this->auth_session->username);
		if (! $this->acl->isAllowed($this->user_role, 'clients', 'link') && $this->_request->getActionName() != 'unauthorized') {
			$this->_forward('unauthorized');
		}
	}
	/**
	 * The default action
	 */
	public function indexAction ()
	{
		$this->_helper->layout()->setLayout('SimpleBlue');
		$client_auth = new Api_Model_Authenticator();
		$clients_model = new Admin_Model_Clients();
		$list_clients = $clients_model->fetchClients($this->auth_session->username);
		$this->view->api_keys = array();
		for($i = 0; $i < count($list_clients); $i++) {
			$this->view->api_keys[$i][0] = $list_clients[$i]['sys_name'];
			$this->view->api_keys[$i][1] = $client_auth->apiKey($list_clients[$i]['sys_name']);
		}
	}
	public function unauthorizedAction ()
	{
		$this->_helper->layout()->setLayout('SimpleBlue');
		$this->view->assign('user_role', $this->user_role);
	}
}

