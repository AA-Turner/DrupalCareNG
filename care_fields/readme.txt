INTRODUCTION
------------

This module provides fields useful to connect a Drupal site to a CARE CRM system.

The fields store data in a CARE-compatible format, and keep track of whether the
data has been edited in Drupal or not.


INSTALLATION & CONFIGURATION
----------------------------

1) Install module and enable it.
2) Configure the CARE field options, for HTML SELECT fields, at:
   Configuration > Web services > CARE > Field option values.
   /admin/config/services/care/fields
3) Add fields to entities as required.


USAGE
-----

Email, address and telephone fields can be configured as single-value fields in
Drupal, in which case the default/preferred data from CARE will be used for
them. Alternatively, if the fields are configured to have multiple values then
all the data from CARE will be used for them.

The fields do not have any code to read data from CARE, or to write it to CARE.

CREDITS
-------

Development of this module was sponsored by Cycling UK.

 
CONTACT
-------

This module was written by, and is maintained by,
Anthony Cartmell <ajcartmell@fonant.com>
