#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
##									      ##
## This program is free software;  you can redistribute it and/or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	openldap2.sh
#
# Description:	Test openldap2 package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Apr 14 2003 - created - RR
#		06 Jan 2004 - (RR) updated to tc_utils.source
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variables
#
REQUIRED="which cat cmp ls rm sed"

# $OpenLDAP: pkg/ldap/tests/scripts/defines.sh

DATADIR=./openldap2-tests/data
PROGDIR=./openldap2-tests/progs
PASSWDCONF=$DATADIR/slapd-passwd.conf
CLIENTDIR=/usr/bin
SERVERDIR=/usr/sbin

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

SLAPADD="$SERVERDIR/slapadd $LDAP_VERBOSE"
SLAPCAT="$SERVERDIR/slapcat $LDAP_VERBOSE"
SLAPINDEX="$SERVERDIR/slapindex $LDAP_VERBOSE"

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

DBDIR=$TCTMP/test-db
REPLDIR=$TCTMP/test-repl
mkdir -p $DBDIR $REPLDIR

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

################################################################################
# testcase functions
################################################################################

# $OpenLDAP: pkg/ldap/tests/scripts/test000-rootdse

function test000-rootdse {

	echo "Datadir is $DATADIR"
	echo "Cleaning up in $DBDIR..."
	rm -f $DBDIR/*

	echo "Starting slapd on TCP/IP port $PORT..."
	SLAPD -f $SCHEMACONF -h $MASTERURI -d $LVL $TIMING > $MASTERLOG 2>&1 &
	PID=$!

	echo "Using ldapsearch to retrieve all the entries..."
	for i in 0 1 2 3; do
		$LDAPSEARCH -b "" -s base -h localhost:$PORT '+' > $SEARCHOUT 2>&1
		RC=$?
		if test $RC = 1 ; then
			echo "Waiting 5 seconds for slapd to start..."
			sleep 5
		fi
	done

	kill -HUP $PID
	cat $SEARCHOUT

	if test $RC != 0 ; then
		echo ">>>>> Test failed"
	else
		if grep "TLS:" $SEARCHOUT; then
			RC=-1
		else
			echo ">>>>> Test succeeded"
		fi
	fi

	return $RC
}

####################################################################################
# MAIN
####################################################################################

# Function:	main
#
# Description:	- Execute all tests, report results
#
# Exit:		- zero on success
#		- non-zero on failure
#
tc_setup
tc_exec_or_break $REQUIRED || exit
test000-rootdse
