# System Requirements #

## Server ##
The Display UI server is powered by PHP, and requires the following components, at minimum:
  * PHP 5.2.4
    * Hash extension
    * cURL (or sockets)
    * Zip extension
    * PEAR
    * PDO (or MySQLi)
    * Safe mode off
    * Output buffering off
  * Apache 2.2
  * MySQL 5.0

The server software _must be able to write to the filesystem_, especially the `application/configs` and media directories. The locations of both directories can be configured manually.

Additionally, we recommend a setup with the following configuration:
  * Alternative PHP Cache

## Client ##

The Display UI client is built in C/C++ and GDI, and has only been tested with the following prerequisites:


|  | **Minimum Requirements** | **Recommended Requirements** |
|:-|:-------------------------|:-----------------------------|
| **Operating System** | Windows XP or newer |
| **Processor** | 1 GHz | 1.7 GHz or greater |
| **RAM** | 256 MB | 1 GB or greater |
| **Internet Connection** | Dial-up (56.6 Kbps) | Broadband |
| **Screen Resolution** | 1280x720 |
| **Graphics Card** | DirectX support | DirectX 9.0b or better recommended |
| **Other Requirements** | [DejaVu](http://dejavu-fonts.org/wiki/index.php?title=Main_Page) Sans font family | Windows Media Format 11 Runtime or Windows Media Player 11<br />Codecs for H.264 and XviD (we recommend the [Combined Community Codec Pack](http://cccp-project.net/)) |