DROP TABLE IF EXISTS `dui_users`;
CREATE TABLE IF NOT EXISTS `dui_users` (
  `id`			integer PRIMARY KEY AUTOINCREMENT,
  `username`	varchar(64) NOT NULL,
  `password`	varchar(64) NOT NULL,
  `email`		varchar(128) NOT NULL,
  `last_active`	datetime NOT NULL,
  `acl_role`	varchar(64) NOT NULL
);

--
-- Table structure for table `dui_clients`
--

DROP TABLE IF EXISTS `dui_clients`;
CREATE TABLE IF NOT EXISTS `dui_clients` (
  `id`			integer PRIMARY KEY AUTOINCREMENT,
  `sys_name`	varchar(64) NOT NULL,
  `last_active`	datetime NOT NULL,
  `admin`		integer NOT NULL,
  `users`		varchar(64) NOT NULL,
  `location`	varchar(16) NOT NULL
);

--
-- Table structure for table `dui_headlines`
--

DROP TABLE IF EXISTS `dui_headlines`;
CREATE TABLE IF NOT EXISTS `dui_headlines` (
  `id`			integer PRIMARY KEY AUTOINCREMENT,
  `title`		varchar(256) NOT NULL,
  `active`		tinyint(1) NOT NULL,
  `expires`		datetime NOT NULL,
  `type`		varchar(32) NOT NULL,
  `clients`		varchar(32) NOT NULL
);

--
-- Table structure for table `dui_options`
--

DROP TABLE IF EXISTS `dui_options`;
CREATE TABLE IF NOT EXISTS `dui_options` (
  `id`			integer PRIMARY KEY AUTOINCREMENT,
  `option_name`	varchar(64) NOT NULL,
  `option_value` mediumtext NOT NULL,
  `client_id`	integer NOT NULL
);

--
-- Table structure for table `dui_media`
--

DROP TABLE IF EXISTS `dui_media`;
CREATE TABLE IF NOT EXISTS `dui_media` (
  `id`			integer PRIMARY KEY AUTOINCREMENT,
  `title`		varchar(64) NOT NULL,
  `activates`	datetime NOT NULL,
  `expires`		datetime NOT NULL,
  `active`		tinyint NOT NULL,
  `type`		varchar(32) NOT NULL,
  `clients`		varchar(64) NOT NULL,
  `weight`		smallint NOT NULL,
  `content`		varchar(128) NOT NULL,
  `data`		mediumblob NULL
);

--
-- Table structure for table `dui_playlists`
--

DROP TABLE IF EXISTS `dui_playlists`;
CREATE TABLE IF NOT EXISTS `dui_playlists` (
  `id`			integer PRIMARY KEY AUTOINCREMENT,
  `generated`	datetime NOT NULL,
  `revision`	tinyint NOT NULL,
  `client`		integer NOT NULL,
  `played`		datetime NOT NULL,
  `content`		mediumtext NOT NULL
);