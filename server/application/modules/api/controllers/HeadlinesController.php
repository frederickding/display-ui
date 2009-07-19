<?php
/**
 * Headlines API
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
 * The headlines API
 */
class Api_HeadlinesController extends Zend_Controller_Action {
	/**
	 * Only forwards the request to fetchAction()
	 */
	public function indexAction()
	{
		$this->_forward('fetch');
	}
	public function fetchAction()
	{
		$this->_helper->viewRenderer->setNoRender();
		$Authenticator = new Api_Model_Authenticator();
		
		// load our parameters from POST/GET
		$sys_name = $this->_getParam('sys_name');
		$signature = $this->_getParam('sig');
		
		if ($Authenticator->verify($sys_name, $signature)) {
			// we don't need $signature or the Authenticator model anymore
			unset($signature, $Authenticator);
			// get some more variables
			$number = (int) $this->_getParam('number');
			$type = (!empty($_REQUEST['type'])) ? trim($this->_getParam('type')) : NULL;
			// let's prepare the headlines
			$HeadlinesModel = new Api_Model_Headlines();
			$headlines = $HeadlinesModel->fetchConcatenated($number, $sys_name, $type);
			// target format: HEADLINE | HEADLINE | HEADLINE ...
			$this->getResponse()->setHeader('Content-Type', 'text/plain; charset=UTF-16LE', true)
								//->setBody(iconv('UTF-8', 'UTF-16LE', $headlines));
								->setBody($headlines);
		} else {
			$this->getResponse()->setHttpResponseCode(401)
			->setHeader('Content-Type', 'text/plain; charset=UTF-16LE', true)
			->setBody(iconv('UTF-8', 'UTF-16LE', 'Headlines inaccessible; access denied.'));
		}
	}
}
