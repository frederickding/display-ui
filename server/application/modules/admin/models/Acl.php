<?php
/**
 * Acl model for administrative interface
 *
 * Copyright 2009 Frederick Ding<br />
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * or the full licensing terms for this project at
 * http://code.google.com/p/display-ui/wiki/License
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
	public function __construct ()
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
	private function addRoles ()
	{
		$this->acl->addRole(new Zend_Acl_Role('guest')) // no permissions
			->addRole(new Zend_Acl_Role('publisher'), 'guest') // publish content
			->addRole(new Zend_Acl_Role('it'), 'guest') // manage options, users, clients, backup
			->addRole(new Zend_Acl_Role('admin')); // everything
	}
	/**
	 * Sets up all known resources in the admin module
	 */
	private function addResources ()
	{
		$this->acl->addResource(new Zend_Acl_Resource('backend')) // the backend itself
			->addResource(new Zend_Acl_Resource('headlines')) // headline functionality
			->addResource(new Zend_Acl_Resource('multimedia')) // media functionality
			->addResource(new Zend_Acl_Resource('options')) // system options
			->addResource(new Zend_Acl_Resource('users')) // user access
			->addResource(new Zend_Acl_Resource('clients')) // client systems
			->addResource(new Zend_Acl_Resource('backuprestore')); // data management
	}
	/**
	 * Adds allow rules for groups
	 */
	private function addAllows ()
	{
		// all authenticated users can use the backend
		$this->acl->allow(
		array(
			'publisher',
			'it',
			'admin'), 'backend');
		// publisher can view, add, edit and delete; hence no privileges enumerated
		$this->acl->allow('publisher',
		array(
			'headlines',
			'multimedia'));
		// give view-only privilege for clients resource
		$this->acl->allow('publisher', 'clients',
		array(
			'view',
			'list'));
		// IT can view, add, edit, delete options, users and clients
		// IT can also view, make and restore backups
		$this->acl->allow('it',
		array(
			'options',
			'users',
			'clients',
			'backuprestore'));
		// give view-only privilege for headlines and multimedia
		$this->acl->allow('it',
		array(
			'headlines',
			'multimedia'), array(
			'view',
			'list'));
		// admin can do everything
		$this->acl->allow('admin');
	}
	/**
	 * Adds deny rules for groups
	 */
	private function addDenys ()
	{
		// deny publishers all access to backup/restore functionality
		$this->acl->deny('publisher', 'backuprestore', NULL)
			->// deny publishers write access to client system info
		deny('publisher', 'clients',
		array(
			'add',
			'edit',
			'delete',
			'link'))
			->// deny publishers access to users resource
		deny('publisher', 'users', NULL)
			->// deny IT write access to content functionality
		deny('it', array(
			'headlines',
			'multimedia'), array(
			'add',
			'edit',
			'delete'));
	}
	/**
	 * Returns true if given role has access to the specified resource/privilege
	 * @param  Zend_Acl_Role_Interface|string
	 * @param  Zend_Acl_Resource_Interface|string
	 * @param  string
	 * @return boolean
	 */
	public function isAllowed ($_role, $_resource, $_privilege = NULL)
	{
		return $this->acl->isAllowed($_role, $_resource, $_privilege);
	}
	/**
	 * Dumps the ACL, for debugging purposes
	 */
	public function dump ()
	{
		$resources = array(
			'backend',
			'headlines',
			'multimedia',
			'options',
			'users',
			'clients',
			'backuprestore');
		$dump = '';
		foreach ($resources as $resource) {
			$dump .= "PUBLISHER GROUP $resource\n";
			$dump .= $this->acl->isAllowed('publisher', $resource) ? "allowed\n" : "denied\n";
		}
		foreach ($resources as $resource) {
			$dump .= "IT GROUP $resource\n";
			$dump .= $this->acl->isAllowed('it', $resource) ? "allowed\n" : "denied\n";
		}
		foreach ($resources as $resource) {
			$dump .= "ADMIN GROUP $resource\n";
			$dump .= $this->acl->isAllowed('admin', $resource) ? "allowed\n" : "denied\n";
		}
		$dump .= "PUBLISHER GROUP clients.view\n" . ($this->acl->isAllowed(
		'publisher', 'clients', 'view') ? "allowed\n" : "denied\n");
		$dump .= "IT GROUP headlines.view\n" . ($this->acl->isAllowed('it',
		'headlines', 'view') ? "allowed\n" : "denied\n");
		return $dump;
	}
	public function getRoles ()
	{
		return $this->acl->getRoles();
	}
}