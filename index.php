<?php
define('PERFORMANCE_TIMER', microtime(TRUE));

defined('APPLICATION_VER')
	|| define('APPLICATION_VER', '0.5.0-beta');

// Define path to application directory
defined('APPLICATION_PATH')
    || define('APPLICATION_PATH', realpath(dirname(__FILE__) . '/application'));

// START: .htaccess/environmentally configurable constants
// Define application environment
defined('APPLICATION_ENV')
    || define('APPLICATION_ENV',
    	(getenv('APPLICATION_ENV') ? getenv('APPLICATION_ENV') : 'production'));

// Define configuration directory; in the future, may be above public directory
defined('CONFIG_DIR')
	|| define('CONFIG_DIR', (getenv('CONFIG_DIR') ? getenv('CONFIG_DIR') :
		realpath(APPLICATION_PATH . '/configs')));

// Define media storage directory; in the future, may be above public directory
defined('MEDIA_DIR')
	|| define('MEDIA_DIR',
		(getenv('MEDIA_DIR') ? getenv('MEDIA_DIR') :
		realpath(dirname(__FILE__) . '/storage/media')));
// END: .htaccess/environmentally configurable constants

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
    CONFIG_DIR . '/application.ini'
);
$application->bootstrap()
            ->run();