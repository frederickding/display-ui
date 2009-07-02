<?php

class IndexController extends Zend_Controller_Action
{

    public function init()
    {
		$this->base_uri = $_SERVER['REQUEST_URI'];
    }

    public function indexAction()
    {
        // action body
		$this->view->base_uri = $this->base_uri;
		
    }


}

