<?php
/**
 * Acl model for administrative interface
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
 * Provides authorization support for admin interface
 */
class Admin_Model_Acl
{
	/**
	 * Stores an instance of the Zend_Acl class
	 * @var Zend_Acl
	 */
	private $acl;
	/**
	 * Constructs the Acl object by setting up roles, resources and rules
	 */
	public function __construct()
	{
		$this->acl = new Zend_Acl();
		$this->addRoles();
		$this->addResources();
		$this->addAllows();
		$this->addDenys();
	}
	/**
	 * Sets up roles
	 */
	private function addRoles()
	{
		$this->acl->addRole(new Zend_Acl_Role('guest')) // no permissions
				  ->addRole(new Zend_Acl_Role('publisher'), 'guest') // publish content
				  ->addRole(new Zend_Acl_Role('it'), 'guest') // manage options, users, clients, backup
				  ->addRole(new Zend_Acl_Role('admin')); // everything
	}
	/**
	 * Sets up all known resources in the admin module
	 */
	private function addResources()
	{
		$this->acl->add(new Zend_Acl_Resource('backend')) // the backend itself
				  ->add(new Zend_Acl_Resource('headlines')) // headline functionality
				  ->add(new Zend_Acl_Resource('multimedia')) // media functionality
				  ->add(new Zend_Acl_Resource('options')) // system options
				  ->add(new Zend_Acl_Resource('users')) // user access
				  ->add(new Zend_Acl_Resource('clients')) // client systems
				  ->add(new Zend_Acl_Resource('backuprestore')); // data management
	}
	/**
	 * Adds allow rules for groups
	 */
	private function addAllows()
	{
		$this->acl->allow(array('publisher','it','admin'), 'backend');
		$this->acl->allow('publisher', array(
			'headlines',
			'multimedia'
		));
		$this->acl->allow('publisher', 'clients', 'view');
		$this->acl->allow('it', array(
			'options',
			'users',
			'clients',
			'backuprestore'
		));
		$this->acl->allow('it', array('headlines', 'multimedia'), 'view');
		$this->acl->allow('admin');
	}
	/**
	 * Adds deny rules for groups
	 */
	private function addDenys()
	{
		$this->acl->deny('publisher', 'backuprestore', NULL)
				  ->deny('publisher', 'clients', array('add', 'edit', 'delete'))
				  ->deny('publisher', 'users', NULL)
				  ->deny('it', array('headlines', 'multimedia'), array('add', 'edit', 'delete'));
	}
	/**
	 * Returns true if given role has access to the specified resource/privilege
     * @param  Zend_Acl_Role_Interface|string
     * @param  Zend_Acl_Resource_Interface|string
     * @param  string
     * @return boolean
	 */
	public function isAllowed($_role, $_resource, $_privilege = NULL)
	{
		return $this->acl->isAllowed($_role, $_resource, $_privilege);
	}
	/**
	 * Dumps the ACL, for debugging purposes
	 */
	public function dump()
	{
		$resources = array(
			'backend',
			'headlines',
			'multimedia',
			'options',
			'users',
			'clients',
			'backuprestore'
		);
		$dump = '';
		foreach($resources as $resource) {
			$dump .= "PUBLISHER GROUP $resource\n";
			$dump .= $this->acl->isAllowed('publisher', $resource) ? "allowed\n" : "denied\n";
		}
		foreach($resources as $resource) {
			$dump .= "IT GROUP $resource\n";
			$dump .= $this->acl->isAllowed('it', $resource) ? "allowed\n" : "denied\n";
		}
		foreach($resources as $resource) {
			$dump .= "ADMIN GROUP $resource\n";
			$dump .= $this->acl->isAllowed('admin', $resource) ? "allowed\n" : "denied\n";
		}
		return $dump;
	}
}