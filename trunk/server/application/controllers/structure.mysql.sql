-- -----------------------------------------------------------------------------
-- SQL database structure for Display UI Server
-- -----------------------------------------------------------------------------

--
-- Table structure for table `dui_users`
--

DROP TABLE IF EXISTS `dui_users`;
CREATE TABLE IF NOT EXISTS `dui_users` (
  `id`			int(10) unsigned NOT NULL auto_increment,
  `username`	varchar(64) NOT NULL,
  `password`	varchar(64) NOT NULL,
  `email`		varchar(128) NOT NULL,
  `last_active`	datetime NOT NULL,
  `acl_role`	varchar(64) NOT NULL,
  PRIMARY KEY  (`id`,`username`),
  KEY `email` (`email`)
) DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Table structure for table `dui_clients`
--

DROP TABLE IF EXISTS `dui_clients`;
CREATE TABLE IF NOT EXISTS `dui_clients` (
  `id`			int(10) unsigned NOT NULL auto_increment,
  `sys_name`	varchar(64) NOT NULL,
  `last_active`	datetime NOT NULL,
  `admin`		int(10) unsigned NOT NULL,
  `users`		varchar(64) NOT NULL,
  `location`	varchar(16) NOT NULL,
  PRIMARY KEY  (`id`)
) DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Table structure for table `dui_headlines`
--

DROP TABLE IF EXISTS `dui_headlines`;
CREATE TABLE IF NOT EXISTS `dui_headlines` (
  `id`			int(10) unsigned NOT NULL auto_increment,
  `title`		varchar(256) NOT NULL,
  `active`		tinyint(1) NOT NULL,
  `expires`		datetime NOT NULL,
  `type`		varchar(32) NOT NULL,
  `clients`		varchar(32) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `title` (`title`),
  KEY `type` (`type`)
) DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Table structure for table `dui_options`
--

DROP TABLE IF EXISTS `dui_options`;
CREATE TABLE IF NOT EXISTS `dui_options` (
  `id`			int(10) unsigned NOT NULL auto_increment,
  `option_name`	varchar(64) NOT NULL,
  `option_value` mediumtext NOT NULL,
  `client_id`	int(10) unsigned NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `option_name` (`option_name`)
) DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Table structure for table `dui_media`
--

DROP TABLE IF EXISTS `dui_media`;
CREATE TABLE IF NOT EXISTS `dui_media` (
  `id`			int(10) unsigned NOT NULL auto_increment,
  `title`		varchar(64) NOT NULL,
  `activates`	datetime NOT NULL,
  `expires`		datetime NOT NULL,
  `active`		tinyint(1) NOT NULL,
  `type`		varchar(32) NOT NULL,
  `clients`		varchar(64) NOT NULL,
  `weight`		smallint NOT NULL,
  `content`		varchar(128) NOT NULL,
  `data`		mediumblob NULL,
  PRIMARY KEY (`id`),
  KEY `title` (`title`)
) DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- Table structure for table `dui_playlists`
--

DROP TABLE IF EXISTS `dui_playlists`;
CREATE TABLE IF NOT EXISTS `dui_playlists` (
  `id`			int(10) unsigned NOT NULL auto_increment,
  `generated`	datetime NOT NULL,
  `revision`	tinyint NOT NULL,
  `client`		int(10) unsigned NOT NULL,
  `played`		datetime NOT NULL,
  `content`		mediumtext NOT NULL,
  PRIMARY KEY (`id`),
  KEY `client` (`client`)
) DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;