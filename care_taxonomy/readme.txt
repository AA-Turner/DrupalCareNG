INTRODUCTION
------------

This module imports various taxonomies into a Drupal site from a CARE CRM system.

Vocabularies imported are:

 * Source codes (xldtSources with LookupGroup = "WEB")
 * Promotion codes (xldtSources)
 * Membership Codes (xldtMembershipTypes)
 * Mailing Suppression Codes (xldtSuppressions)

REQUIREMENTS
------------

 * CARE module
 * Taxonomy module
 * Term Status module (optional but recommended)

If the termstatus module is enabled then this module will use it to unpublish terms
that are no longer found in CARE.

INSTALLATION & CONFIGURATION
----------------------------

1) Install module and enable it.
2) Optionally also install and enable the termstatus module.
3) Import taxonomy terms at:
   Structure > Taxonomy > CARE integration
   /admin/structure/taxonomy/care


USAGE
-----

As well as importing taxonomy terms on manual request, terms will also be imported once per day between 3am and 4am.


CREDITS
-------

Development of this module was sponsored by Cycling UK (formerly known as CTC,
the UK's national cycling charity).


CONTACT
-------

This module was written by, and is maintained by,
Anthony Cartmell <ajcartmell@fonant.com>
