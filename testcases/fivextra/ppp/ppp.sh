#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2001		      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	ppp.sh
#
# Description:	Test ppp program (using pppoe).
#
# Author:	Amos Waterland <apw@us.ibm.com>
#
# History:	Jun 27 2003 - Created.
#		Jul 02 2003 - RC Paulsen modified to use $TCTMP and to use
#				the "executes" function.
#		Oct 08 2003 - RC Paulsen: new location for ifcfg-ppp0. Also
#				don't require the file to pre-exist.
#				Use variables to hold config file names.
#				Use new local setup/cleanup scheme.
#		05 Jan 2004 - (robert) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# identify the config files
chap_secrets=/etc/ppp/chap-secrets
pap_secrets=/etc/ppp/pap-secrets
pppoe_conf=/etc/pppoe.conf

# which ethernet device to use
eth=eth0

bring_down_ppp0="no"

################################################################################
# local utility functions
################################################################################

# Write our conf files, saving originals to be restored by pop().
#
function push_pppoe_conf()
{
	#first save the original files
        [ -e $chap_secrets ] && {
		cp $chap_secrets $TCTMP/chap-secrets || return 1;
	}

        [ -e $pap_secrets ] && {
		cp $pap_secrets $TCTMP/psp-secrets || return 1;
	}

        [ -e $pppoe_conf ] && {
		cp $pppoe_conf $TCTMP/pppoe.conf || return 1;
	}

        #only send output to stdout, to ignore spurious errors
	tc_info "piping answers to adsl-setup wizard ..."
        printf "root\n$eth\nno\n\n\n\n0\ny\n" | adsl-setup &>$stdout ||
        {
            tc_break_if_bad 1 "adsl-setup failed, bailing out";
            return 1;
	}

	return 0;
}

# undo push()
#
function pop_pppoe_conf()
{
	[ -e $TCTMP/chap-secrets ] &&
		cp $TCTMP/chap-secrets $chap_secrets
	[ -e $TCTMP/pap-secrets ] &&
		cp $TCTMP/pap-secrets $pap_secrets
	[ -e $TCTMP/pppoe.conf ] &&
		cp $TCTMP/pppoe.conf $pppoe_conf
}

# cleanup after ourselves
#
function tc_local_cleanup()
{
	#restore the pppoe conf files that we modified
	pop_pppoe_conf

	#bring down ppp0
	[ "$bring_down_ppp0" = "yes" ] && adsl-stop >$stdout 2>$stderr
}

function tc_local_setup()
{
	tc_exec_or_break ifconfig grep pppoe || return

	#chicken out if ppp0 is already up
	ifconfig | grep ppp0 >$stdout 2>$stderr && {
		tc_break_if_bad 1 "can't run test. ppp0 already up".
		return 1
	}

	#we require pppoe server to be running for this test to work
	tc_info "Looking for required pppoe server ..."
	pppoe -d >$stdout 2>$stderr || {
		tc_break_if_bad 1 "... pppoe server not available"
		return 1
	}
	tc_info "pppoe server discovery succeeded"

        #write the conf files we want
        push_pppoe_conf || return 1
}

################################################################################
# the testcase functions
################################################################################

function test01()
{
	tc_register "installation check"

	tc_executes pppd
	tc_fail_if_bad $? "pppd not installed properly (pppd missing)" || return

	tc_exist_or_break	$chap_secrets $pap_secrets
	tc_pass_or_fail $? "pppd not installed properly (missing config files)"
}

function test02()
{
	tc_register "check that ppp over ethernet works"
	tc_exec_or_break cat adsl-start adsl-stop ifconfig awk ping grep || return

        local peer;
        #now try to bring up the ppp0 interface
	adsl-start >$stdout 2>$stderr
	tc_fail_if_bad $? "could not bring up ppp0 interface" || return

	bring_down_ppp0="yes"	# for tc_local_cleanup

        #now verify that the ppp0 interface is listed by ifconfig
	ifconfig | grep -q ppp0
	tc_fail_if_bad $? "ppp0 interface not listed by ifconfig" || return

        #now extract IP address of our ppp peer (machine running ppp-server)
        peer=`ifconfig ppp0 |
               awk -F: '/inet addr/ {print $3}' |
                awk '{print $1}'`
	tc_fail_if_bad $? "could not get peer's IP address" || return

        #now try to ping our peer (this really should work)
	ping -c 1 $peer >$stdout 2>$stderr
	tc_fail_if_bad $? "Could not ping peer ($peer)" || return

	#now bring down the ppp0 interface
	adsl-stop >$stdout 2>$stderr
	tc_pass_or_fail $? "error bringing down ppp0 interface"
}

################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup			# standard setup

tc_root_or_break || exit

test01 &&
test02
