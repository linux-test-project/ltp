#! /bin/sh
# $OpenLDAP: pkg/ldap/tests/scripts/makeldbm.sh,v 1.4.8.3 2000/06/13 17:57:44 kurt Exp $

. defines.sh

echo "Cleaning up in $DBDIR..."

rm -f $DBDIR/[!C]*

echo "Running slapadd to build slapd database..."
$SLAPADD -f $CONF -l $LDIF
RC=$?
if test $RC != 0 ; then
	echo "slapadd failed!"
	exit $RC
fi
