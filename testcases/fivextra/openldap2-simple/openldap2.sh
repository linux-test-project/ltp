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

################################################################################
# testcase functions
################################################################################

function test01 {

	local SLAPD="/usr/lib/openldap/slapd"
	local SLAPADD="/usr/sbin/slapadd"
	local CONF=slapd.conf
	local LDIF=big.ldif
	local TESTF=test.ldif

	tc_register "startldap"
	/etc/init.d/ldap start 2>$stderr 1>$stdout
	tc_pass_or_fail $? "Unable to start the ldap server"

	tc_register "ldapsearch"
	tc_info "waiting for ldap server to stabilize"
	sleep 3
	ldapsearch -x -b '' -s base '(objectclass=*)' namingContexts 1>$stdout 2>$stderr
	tc_pass_or_fail $? "ldapsearch failed"
	
	tc_register "stopldap"
	/etc/init.d/ldap stop 2>$stderr 1>$stdout
	tc_pass_or_fail $? "Unable to stop the ldap server"
	sleep 3

################################

#	tc_info "cleaning out existing database entries"
#	rm -rf openldap-data/*

#	tc_register "myslapadd"
#	$SLAPADD -f $CONF -l $LDIF
#	tc_pass_or_fail $? "myslapadd failed"

#	tc_register "startmyldap"
#	$SLAPD -f $CONF -h ldap://localhost:9001/ 1>/dev/null 2>$stderr &
#	tc_pass_or_fail $? "unable to start local server"
	
#	tc_info "waiting for local ldap server to stabilize"
#	sleep 3

#	tc_register "myldapsearch"
#	ldapsearch -h localhost -p 9001 -x -b '' -s base '(objectclass=*)' namingContexts 1>/dev/null 2>$stderr
#	tc_pass_or_fail $? "myldapsearch failed"

#	tc_register "myldapadd"
#	ldapadd -x -D "cn=Manager,dc=my-domain,dc=com" -H ldap://localhost:9001/ -f test.ldif
#	ldapadd -x -H ldap://localhost:9001/ -f test.ldif
#	ldapadd -w secret -f $LDIF -h localhost -p 9001 1>/dev/null 2>$stderr
#	tc_pass_or_fail $? "myldapadd failed"

#	killall slapd &>/dev/null
	
}
####################################################################################
# MAIN
####################################################################################

# Function:	main
#

#
# Exit:		- zero on success
#		- non-zero on failure
#
TST_TOTAL=1
tc_setup
tc_root_or_break || exit
tc_exec_or_break $REQUIRED || exit
mkdir -p openldap-data || exit
test01
