<?php

/**
 * IndexController for API
 *
 * @author Frederick
 * @version $Id$
 */

class IndexController extends Zend_Controller_Action {
	/**
	 * The default action - show the home page
	 */
	public function indexAction() {
		$this->getResponse()->setHttpResponseCode(400);
	}

}