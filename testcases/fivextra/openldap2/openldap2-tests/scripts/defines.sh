#! /bin/sh
# $OpenLDAP: pkg/ldap/tests/scripts/defines.sh,v 1.27.2.6 2000/10/30 18:14:15 kurt Exp $

DATADIR=./data
PROGDIR=./progs

if test "$BACKEND" = "bdb2" ; then
	CONF=$DATADIR/slapd-bdb2-master.conf
	PWCONF=$DATADIR/slapd-bdb2-pw.conf
	ACLCONF=$DATADIR/slapd-bdb2-acl.conf
	MASTERCONF=$DATADIR/slapd-bdb2-repl-master.conf
	SLAVECONF=$DATADIR/slapd-bdb2-repl-slave.conf
	REFSLAVECONF=$DATADIR/slapd-bdb2-ref-slave.conf
	SCHEMACONF=$DATADIR/slapd-bdb2-schema.conf
	TIMING="-t"
else
	CONF=$DATADIR/slapd.conf
	MCONF=$DATADIR/slapd-master.conf
	PWCONF=$DATADIR/slapd-pw.conf
	ACLCONF=$DATADIR/slapd-acl.conf
	MASTERCONF=$DATADIR/slapd-repl-master.conf
	SLAVECONF=$DATADIR/slapd-repl-slave.conf
	REFSLAVECONF=$DATADIR/slapd-ref-slave.conf
	SCHEMACONF=$DATADIR/slapd-schema.conf
fi

TOOLARGS="-x $LDAP_TOOLARGS"
TOOLPROTO="-P 3"

PASSWDCONF=$DATADIR/slapd-passwd.conf

CLIENTDIR=/usr/bin

SLAPADD="/usr/sbin/slapadd $LDAP_VERBOSE"
SLAPCAT="/usr/sbin/slapcat $LDAP_VERBOSE"
SLAPINDEX="/usr/sbin/slapindex $LDAP_VERBOSE"

CMP="diff -i"
CMPOUT=/dev/null
SLAPD=/usr/lib/openldap/slapd
SLURPD=/usr/lib/openldap/slurpd
LDAPPASSWD="$CLIENTDIR/ldappasswd $TOOLARGS"
LDAPSEARCH="$CLIENTDIR/ldapsearch $TOOLPROTO $TOOLARGS -LLL"
LDAPMODIFY="$CLIENTDIR/ldapmodify $TOOLPROTO $TOOLARGS"
LDAPADD="$CLIENTDIR/ldapadd $TOOLPROTO $TOOLARGS"
LDAPMODRDN="$CLIENTDIR/ldapmodrdn $TOOLPROTO $TOOLARGS"
SLAPDTESTER=$PROGDIR/slapd-tester
LVL=${SLAPD_DEBUG-5}
ADDR=127.0.0.1
PORT=9009
SLAVEPORT=9010
MASTERURI="ldap://localhost:$PORT/"
SLAVEURI="ldap://localhost:$SLAVEPORT/"

DBDIR=./test-db
REPLDIR=./test-repl
LDIF=$DATADIR/test.ldif
LDIFORDERED=$DATADIR/test-ordered.ldif
LDIFPASSWD=$DATADIR/passwd.ldif
LDIFPASSWDOUT=$DATADIR/passwd-out.ldif
MONITOR=""
BASEDN="o=University of Michigan, c=US"
MANAGERDN="cn=Manager, o=University of Michigan, c=US"
UPDATEDN="cn=Replica, o=University of Michigan, c=US"
PASSWD=secret
BABSDN="cn=Barbara Jensen, ou=Information Technology Division, ou=People, o=University of  Michigan , c = US "
BJORNSDN="cn=Bjorn Jensen, ou=Information Technology Division, ou=People, o=University of Michigan, c=US"
JAJDN="cn=James A Jones 1, ou=Alumni Association, ou=People, o=University of Michigan, c=US"
MASTERLOG=$DBDIR/master.log
SLAVELOG=$DBDIR/slave.log
SLURPLOG=$DBDIR/slurp.log
SEARCHOUT=$DBDIR/ldapsearch.out
SEARCHFLT=$DBDIR/ldapsearch.flt
LDIFFLT=$DBDIR/ldif.flt
MASTEROUT=$DBDIR/master.out
SLAVEOUT=$DBDIR/slave.out
TESTOUT=$DBDIR/test.out
INITOUT=$DBDIR/init.out
SEARCHOUTMASTER=$DATADIR/search.out.master
MODIFYOUTMASTER=$DATADIR/modify.out.master
ADDDELOUTMASTER=$DATADIR/adddel.out.master
MODRDNOUTMASTER0=$DATADIR/modrdn.out.master.0
MODRDNOUTMASTER1=$DATADIR/modrdn.out.master.1
MODRDNOUTMASTER2=$DATADIR/modrdn.out.master.2
MODRDNOUTMASTER3=$DATADIR/modrdn.out.master.3
ACLOUTMASTER=$DATADIR/acl.out.master
REPLOUTMASTER=$DATADIR/repl.out.master
MODSRCHFILTERS=$DATADIR/modify.search.filters
# Just in case we linked the binaries dynamically
#LD_LIBRARY_PATH=`pwd`/../libraries:${LD_LIBRARY_PATH} export LD_LIBRARY_PATH
