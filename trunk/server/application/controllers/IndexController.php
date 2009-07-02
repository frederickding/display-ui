<?php

class IndexController extends Zend_Controller_Action
{

    public function init()
    {
		if(strpos($_SERVER['REQUEST_URI'], 'index')===FALSE) {
			$this->base_uri = rtrim($_SERVER['REQUEST_URI'],'/');
		} else {
			$this->base_uri = explode('/index', $_SERVER['REQUEST_URI']);
			$this->base_uri = $this->base_uri[0];
		}
    }

    public function indexAction()
    {
        // action body
		$this->view->base_uri = $this->base_uri;
		
    }


}

