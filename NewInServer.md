# Introduction #

This document _summarizes_ the **new features** introduced in new versions of the server. It is not a comprehensive changelog; the [commit log](http://code.google.com/p/display-ui/source/list) contains the details of every committed change. Instead, this document should give you the reasons you need to upgrade to a newer version.

# What's New in Display UI Server #
## Version 0.2 series ##
This is the first published version of the server; we began tagging alpha releases of the server beginning at version 0.2. It includes a partially functional administrative interface and a full API (not yet documented) compatible with the client in the 0.3.5 series.
### Version 0.2.4alpha ###
  * It is now possible to **view** image items on the list of multimedia through the "View" link.
  * User experience on the multimedia upload form is improved by attaching descriptions/hints to the form fields that need them, instead of a list at the top of the page.
  * There is now **a form to add headlines** in the headlines management section, more flexible than QuickLine — expiry and category can be set.
## Version 0.4 series ##
These versions add significant new functionality.
### Version 0.4.0-alpha ###
  * A Calendar has been implemented, and simple events can be added through the control panel.
  * Yubikey one-time password security has been implemented.
### Version 0.4.1-alpha ###
  * ZIP archive support added to show a set of images together, in alphabetical order of the filenames.