-- upgrade script to bring pre-0.4.1 to 0.5.0 database structure

SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL';

-- add alternating flags to headlines & media
ALTER TABLE `dui_headlines` ADD COLUMN `alternating` TINYINT(3) UNSIGNED NULL DEFAULT NULL COMMENT 'If set to 1 or 2, specifies the date group (alternating) on which this will be shown.'  AFTER `client` ;
ALTER TABLE `dui_media` ADD COLUMN `alternating` TINYINT(3) UNSIGNED NULL DEFAULT NULL  AFTER `data` ;

-- allow null client_id in options for systemwide options
ALTER TABLE `dui_options` CHANGE COLUMN `client_id` `client_id` INT(10) UNSIGNED NULL DEFAULT NULL  ;

-- add exceptions table
-- -----------------------------------------------------
-- Table `duix_alternating_exceptions`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `duix_alternating_exceptions` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `date` DATE NOT NULL ,
  `comment` VARCHAR(45) NULL COMMENT 'PA Day? Holiday?' ,
  PRIMARY KEY (`id`) ,
  UNIQUE INDEX `date_UNIQUE` (`date` ASC) )
ENGINE = InnoDB;

SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
