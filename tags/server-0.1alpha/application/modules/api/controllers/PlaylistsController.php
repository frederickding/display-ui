<?php
/**
 * Playlists API, which allows retrieval of playlists as well as their creation
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
 * The playlists API
 */
class Api_PlaylistsController extends Zend_Controller_Action
{
	/**
	 * 400 error on index action
	 *
	 * For bad API implementations, indicates that the fetch action should be called;
	 * a 300-series redirect is possible but not preferred, and the _forward()
	 * internal redirect is also not used.
	 */
	public function indexAction()
	{
		$this->_helper->viewRenderer->setNoRender();
		$this->getResponse()->setHttpResponseCode(400)
		->setHeader('Content-Type', 'text/plain', TRUE)
		->setBody('API method does not exist; try /api/playlists/fetch/');
	}
	/**
	 * Fetches an unplayed playlist from the database
	 *
	 * If this is not possible, redirects the client to the generate action.
	 */
	public function fetchAction()
	{
		$this->_helper->viewRenderer->setNoRender();
		$Authenticator = new Api_Model_Authenticator();
		
		// load our parameters from POST/GET
		$sys_name = $this->_getParam('sys_name');
		$signature = $this->_getParam('sig');
		
		if ($Authenticator->verify($sys_name, $signature)) {
			// we don't need the Authenticator model anymore
			unset($Authenticator);
			// let's prepare the playlists
			$PlaylistsModel = new Api_Model_Playlists();
			// get the data
			$raw_data = $PlaylistsModel->fetch($sys_name);
			// if it was not successfully fetched, generate a playlist instead
			if($raw_data === FALSE) {
				$this->_helper->redirector->gotoUrlAndExit(
					'api/playlists/generate/?sys_name='.$sys_name.'&sig='.$signature,
					array('code' => 307));
			}
			// mark it as played in the db
			$PlaylistsModel->updatePlayed($raw_data[0]);
			$this->getResponse()->setHeader('Content-Type', 'application/x-dui-playlist', TRUE)
			->setHeader('Content-Disposition', 'attachment; filename=playlist.dat', TRUE)
			->setBody($PlaylistsModel->buildBinary($raw_data[2]));
		} else {
			$this->getResponse()->setHttpResponseCode(401)
			->setHeader('Content-Type', 'text/plain', TRUE)
			->setBody('Playlists inaccessible; access denied.');
		}
	}
	public function generateAction()
	{
		$this->_helper->viewRenderer->setNoRender();
		$Authenticator = new Api_Model_Authenticator();
		
		// load our parameters from POST/GET
		$sys_name = $this->_getParam('sys_name');
		$signature = $this->_getParam('sig');
		
		if ($Authenticator->verify($sys_name, $signature)) {
			// we don't need $signature or the Authenticator model anymore
			unset($signature, $Authenticator);
			// let's prepare the playlists
			$PlaylistsModel = new Api_Model_Playlists();
			$playlist_id = $PlaylistsModel->generatePlaylist($sys_name);
			// get the data
			$raw_data = $PlaylistsModel->fetch($sys_name, $playlist_id);
			// mark it as played in the db
			$PlaylistsModel->updatePlayed($raw_data[0]);
			$this->getResponse()->setHeader('Content-Type', 'application/x-dui-playlist', TRUE)
			->setHeader('Content-Disposition', 'attachment; filename=playlist.dat', TRUE)
			->setBody($PlaylistsModel->buildBinary($raw_data[2]));
		} else {
			$this->getResponse()->setHttpResponseCode(401)
			->setHeader('Content-Type', 'text/plain', TRUE)
			->setBody('Playlists inaccessible; access denied.');
		}
	}
}