<?php
/**
 * Media API
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
 * The media API provides methods for retrieving media
 */
class Api_MediaController extends Zend_Controller_Action
{
	/**
	 * 400 error on index action
	 *
	 * For bad API implementations, indicates that the download action should be called;
	 * a 300-series redirect is possible but not preferred, and the _forward()
	 * internal redirect is also not used.
	 */
	public function indexAction ()
	{
		$this->_helper->viewRenderer
			->setNoRender();
		$this->getResponse()
			->setHttpResponseCode(400)
			->setHeader('Content-Type', 'text/plain', TRUE)
			->setBody('API method does not exist; try /api/media/download/');
	}
	public function downloadAction ()
	{
		$this->_helper->viewRenderer
			->setNoRender();
		$Authenticator = new Api_Model_Authenticator();
		// load our parameters from POST/GET
		$sys_name = $this->_getParam('sys_name');
		$signature = $this->_getParam('sig');
		$medium = $this->_getParam('id');
		if ($Authenticator->verify($sys_name, $signature)) {
			// we don't need the Authenticator model anymore
			unset($Authenticator);
			$MediaModel = new Api_Model_Media();
			$isStoredDb = $MediaModel->isStoredDb($medium);
			if ($isStoredDb === 1) {
				// get it from the database
				$query = $MediaModel->retrieveFromDb($medium);
				$this->getResponse()
					->setHeader('Content-Type', $query['mime'], TRUE)
					->setHeader('Content-Length', $query['filesize'], TRUE)
					->setHeader('Content-Disposition', 'attachment; filename="' . $query['filename'] . '"', TRUE)
					->setHeader('X-DUI-File-Source', 'database')
					->setBody($query['data']);
			} elseif ($isStoredDb === 0) {
				$query = $MediaModel->retrieveFromFile($medium);
				if ($query !== FALSE) {
					// get it from the filesystem
					$this->getResponse()
						->setHeader('Content-Type', $query['mime'], TRUE)
						->setHeader('Content-Length', $query['filesize'], TRUE)
						->setHeader('Content-Disposition', 'attachment; filename="' . basename($query['filename']) . '"', TRUE)
						->setHeader('X-DUI-File-Source', 'filesystem')
						->sendHeaders();
					$this->getResponse()
						->clearBody();
					readfile($query['filename']);
					die();
				} else {
					$this->getResponse()
						->setHttpResponseCode(404)
						->setHeader('Content-Type', 'text/plain', TRUE)
						->setBody('The media item could not be located on the filesystem.');
				}
			} else {
				$this->getResponse()
					->setHttpResponseCode(404)
					->setHeader('Content-Type', 'text/plain', TRUE)
					->setBody('The media item could not be found.');
			}
		} else {
			$this->getResponse()
				->setHttpResponseCode(401)
				->setHeader('Content-Type', 'text/plain', TRUE)
				->setBody('Media inaccessible; access denied.');
		}
	}
}
