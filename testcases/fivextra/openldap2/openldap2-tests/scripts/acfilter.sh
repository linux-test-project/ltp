#! /bin/sh
# $OpenLDAP: pkg/ldap/tests/scripts/acfilter.sh,v 1.4.8.3 2000/07/04 17:59:01 kurt Exp $
#
# Strip comments
#
egrep -iv '^#'
