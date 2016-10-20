INTRODUCTION
------------

This module provides utility functions to connect a Drupal site to a CARE CRM
system.

Functions include a low-level way to call CARE API methods directly, as well as
some higher-level functions and utilities.

REQUIREMENTS
------------

The PHP SOAP extension needs to be enabled, to allow for SOAP calls to be made
to CARE.

INSTALLATION & CONFIGURATION
----------------------------

1) Install module and enable it.
2) Configure the CARE WSDL, and documentation URLS, at:
   Configuration > Web services > CARE
   /admin/config/services/care


USAGE
-----

This module can run test CARE calls, via the administration menu item:
  Configuration > Web services > CARE > Test CARE call
  /admin/config/services/care/call
  
Other than this, the module does nothing on its own. It may be required by other
modules that need to use connections to a CARE server.


CREDITS
-------

Development of this module was sponsored by Cycling UK (formerly known as CTC,
the UK's national cycling charity).

 
CONTACT
-------

This module was written by, and is maintained by,
Anthony Cartmell <ajcartmell@fonant.com>