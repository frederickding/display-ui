<?php
/**
 * Authentication model for administrative interface
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
 * Provides authentication logic for admin interface
 */
class Admin_Model_Authentication extends Default_Model_DatabaseAbstract
{
	/**
	 * An instance of the PasswordHash class
	 * @var PasswordHash
	 */
	private $PasswordHash;
	/**
	 * Sets up the PasswordHash class and the database
	 */
	public function __construct ()
	{
		$this->PasswordHash = new Default_Model_PasswordHash(8, FALSE);
		parent::__construct();
	}
	/**
	 * Checks whether the provided user's password is valid
	 * @param string $_user
	 * @param string $_password
	 * @return boolean
	 */
	public function checkPassword ($_user, $_password)
	{
		$query = $this->db->quoteInto(
		'SELECT password FROM dui_users WHERE username = ? LIMIT 1', $_user);
		$result = $this->db->fetchOne($query);
		return $this->PasswordHash->CheckPassword($_password, $result);
	}
	public function checkYubikey ($_user, $_otp)
	{
		require_once 'Auth/Yubico.php';
		$yubi = new Auth_Yubico(4566, 'Z4bAzdALPjtAATSYPvVlBalP+jM=');
		// check for the public ID in the db
		$query = $this->db->quoteInto(
		'SELECT yubikey_public FROM dui_users WHERE username = ? LIMIT 1',
		$_user);
		$yubikeyPublicId = $this->db->fetchOne($query);
		$otpParts = $yubi->parsePasswordOTP($_otp);
		// if it's null, no yubikey needed
		if (is_null($yubikeyPublicId))
			return true;
				// if it's not null, compare the prefix of the OTP to the public ID
		else
			if ($otpParts['prefix'] != $yubikeyPublicId)
				return false;
				// all other steps passed, check against Yubico servers
		$auth = $yubi->verify($_otp);
		if (PEAR::isError($auth)) {
			return false;
		} else
			return true;
	}
	/**
	 * Checks whether a given user exists
	 * @param string $_user
	 * @return boolean|int On true, returns User ID
	 */
	public function userExists ($_user)
	{
		$query = $this->db->quoteInto(
		'SELECT id FROM dui_users WHERE username = ? LIMIT 1', $_user);
		$result = $this->db->fetchOne($query);
		if (! empty($result))
			return (int) $result;
		else
			return FALSE;
	}
	/**
	 * Updates a user's password
	 * @param string $_user
	 * @param string $_oldpass
	 * @param string $_newpass
	 * @return boolean
	 */
	public function changePassword ($_user, $_oldpass, $_newpass)
	{
		// $result1 should have the user ID if old pass matches
		// or boolean false if old password is invalid
		$result1 = $this->checkPassword($_user, $_oldpass);
		if ($result1) {
			$query = array(
				'password' => $this->PasswordHash->HashPassword($_newpass),
				'last_active' => new Zend_Db_Expr('UTC_TIMESTAMP()'));
			return $this->db->update('dui_users', $query, 'id = ' . $result1);
		} else
			return FALSE;
	}
	/**
	 * Finds the ACL role of the given user
	 * @param string|int $_user
	 * @return string one of 'publisher', 'it' or 'admin', or when not found, ''
	 */
	public function userRole ($_user)
	{
		if (is_int($_user)) {
			$query = $this->db->quoteInto(
			'SELECT acl_role FROM dui_users WHERE id = ? LIMIT 1', $_user);
		} else {
			$query = $this->db->quoteInto(
			'SELECT acl_role FROM dui_users WHERE username = ? LIMIT 1', $_user);
		}
		$result = $this->db->fetchOne($query);
		if (! empty($result))
			return $result;
		else
			return '';
	}
	public function updateLastActive ($_user)
	{
		if (is_int($_user)) {
			$query = $this->db->update('dui_users',
			array(
				'last_active' => new Zend_Db_Expr('UTC_TIMESTAMP()')),
			$this->db->quoteInto('id = ?', $_user));
		} else {
			$query = $this->db->update('dui_users',
			array(
				'last_active' => new Zend_Db_Expr('UTC_TIMESTAMP()')),
			$this->db->quoteInto('username = ?', $_user));
		}
		return $query;
	}
	public function insertUser ($_user, $_password, $_email, $_role,
	$_yubikey = null)
	{
		if (! in_array($_role, array(
			'admin',
			'publisher',
			'it'))) {
			return false;
		}
		try {
			$insert = array(
				'username' => $_user,
				'password' => $this->PasswordHash->HashPassword($_password),
				'email' => $_email,
				'last_active' => new Zend_Db_Expr('UTC_TIMESTAMP()'),
				'acl_role' => $_role);
			if (! empty($_yubikey)) {
				$insert['yubikey_public'] = $_yubikey;
			}
			$this->db->insert('dui_users', $insert);
		} catch (Zend_Db_Exception $e) {
			return false;
		}
		return $this->db->lastInsertId();
	}
}