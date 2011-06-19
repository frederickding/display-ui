SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL';


-- -----------------------------------------------------
-- Table `dui_users`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `dui_users` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
  `username` VARCHAR(64) NOT NULL ,
  `password` VARCHAR(64) NOT NULL ,
  `email` VARCHAR(128) NOT NULL ,
  `last_active` DATETIME NOT NULL ,
  `acl_role` VARCHAR(64) NOT NULL ,
  `yubikey_public` CHAR(12) NULL ,
  PRIMARY KEY (`id`, `username`) ,
  INDEX `email` (`email` ASC) ,
  UNIQUE INDEX `yubikey_public_UNIQUE` (`yubikey_public` ASC) )
AUTO_INCREMENT = 1
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `dui_clients`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `dui_clients` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
  `sys_name` VARCHAR(64) NOT NULL ,
  `last_active` DATETIME NOT NULL ,
  `admin` INT(10) UNSIGNED NOT NULL ,
  `users` VARCHAR(64) NOT NULL ,
  `location` VARCHAR(16) NOT NULL ,
  PRIMARY KEY (`id`) )
AUTO_INCREMENT = 1
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `dui_headlines`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `dui_headlines` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
  `title` VARCHAR(256) NOT NULL ,
  `active` TINYINT(1) NOT NULL ,
  `expires` DATETIME NOT NULL ,
  `type` VARCHAR(32) NOT NULL ,
  `client` INT(10) UNSIGNED NOT NULL ,
  `alternating` TINYINT UNSIGNED NULL COMMENT 'If set to 1 or 2, specifies the date group (alternating) on which this will be shown.' ,
  PRIMARY KEY (`id`) ,
  INDEX `title` (`title` ASC) ,
  INDEX `type` (`type` ASC) )
AUTO_INCREMENT = 1
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `dui_options`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `dui_options` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
  `option_name` VARCHAR(64) NOT NULL ,
  `option_value` MEDIUMTEXT NOT NULL ,
  `client_id` INT(10) UNSIGNED NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `option_name` (`option_name` ASC) )
AUTO_INCREMENT = 1
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `dui_media`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `dui_media` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
  `title` VARCHAR(64) NOT NULL ,
  `activates` DATETIME NOT NULL ,
  `expires` DATETIME NULL DEFAULT NULL ,
  `active` TINYINT(1) NOT NULL ,
  `type` VARCHAR(32) NOT NULL ,
  `clients` VARCHAR(64) NULL DEFAULT NULL ,
  `weight` SMALLINT NOT NULL DEFAULT 1 ,
  `content` VARCHAR(128) NOT NULL ,
  `data` MEDIUMBLOB NULL DEFAULT NULL ,
  `alternating` TINYINT UNSIGNED NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `title` (`title` ASC) )
AUTO_INCREMENT = 1
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `dui_playlists`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `dui_playlists` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT ,
  `generated` DATETIME NOT NULL ,
  `revision` TINYINT NOT NULL DEFAULT 1 ,
  `client` INT(10) UNSIGNED NOT NULL ,
  `played` DATETIME NULL DEFAULT NULL ,
  `content` MEDIUMTEXT NOT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `client` (`client` ASC) )
AUTO_INCREMENT = 1
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `dui_calendar`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `dui_calendar` (
  `event_id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `name` VARCHAR(18) NOT NULL ,
  `time` DATETIME NOT NULL ,
  `visible` TINYINT(1)  NOT NULL DEFAULT true ,
  `type` ENUM('once','weekly') NOT NULL DEFAULT 'once' ,
  `client` INT(10) UNSIGNED NULL ,
  PRIMARY KEY (`event_id`) ,
  INDEX `client` (`client` ASC) );



SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

-- -----------------------------------------------------
-- Data for table `dui_options`
-- -----------------------------------------------------
START TRANSACTION;
INSERT INTO `dui_options` (`id`, `option_name`, `option_value`, `client_id`) VALUES (NULL, 'alternating_exceptions', 'a:0:{}', NULL);

COMMIT;
