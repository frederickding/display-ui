<?php
defined('APPLICATION_VER')
	|| define('APPLICATION_VER', 'trunk');

// Define path to application directory
defined('APPLICATION_PATH')
    || define('APPLICATION_PATH', realpath(dirname(__FILE__) . '/application'));

// Define application environment
defined('APPLICATION_ENV')
    || define('APPLICATION_ENV',
    	(getenv('APPLICATION_ENV') ? getenv('APPLICATION_ENV') : 'production'));

// Define configuration directory; in the future, may be above public directory
defined('CONFIG_DIR')
	|| define('CONFIG_DIR', realpath(APPLICATION_PATH . '/configs'));

// Ensure library/ is on include_path
set_include_path(implode(PATH_SEPARATOR, array(
    realpath(APPLICATION_PATH . '/../library'),
    get_include_path(),
)));

/** Zend_Application */
require_once 'Zend/Application.php';

// Create application, bootstrap, and run
$application = new Zend_Application(
    APPLICATION_ENV,
    APPLICATION_PATH . '/configs/application.ini'
);
$application->bootstrap()
            ->run();
