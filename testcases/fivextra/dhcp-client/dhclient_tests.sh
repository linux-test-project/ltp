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
# File :	   dhclient_tests.sh
#
# Description: This program tests basic functionality of dhclient program
#
# Author:	   Manoj Iyer  manjo@mail.utexas.edu
#
# History:	   Sept 02 2003 - created - Manoj Iyer
#		Oct 06  2003 - Modified - dhclinet has different options on 
#			debian redhat and suse. Changing testcase to use
#			UL options
#		Oct 16 2003 - RC Paulsen
#			1. don't confuse this testcase with a running
#				dhclient when checking for a currently
#				running dhclient.
#			2. restart loopback interface when done
#			3. kill the dhcpclient started by this testcase
#		08 Jan 2004 - (RR) updated to tc_utils.source,
#				cleanup, add required executables test.
#		27 Jul 2004 (rcp) restart networking when done.

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

restart=0	   # flag to indicate if dhcp needs to be restarted on exit
REQUIRED="grep ifconfig ifdown ifup killall ps"
#
# Function:	tc_local_setup
#	 
# Description:	perform standard setup operations and also check if commands
#		used by this testcase are installed on the system.
#
# Return:	exits with non-zero on failure.
#
tc_local_setup()
{	
	# check dependencies
	tc_root_or_break || exit
	tc_exec_or_break  dhclient diff awk tail cat killall || exit

	# Stop any currently running dhclient.
	# We need this convoluted check due to bug in "ps -C" ...
	# It reports the name of this testcase even though asked
	# for dhclient process.
	name=${0#./}
	name=${name%.*}
	ps -C dhclient | grep dhclient | grep -v $name && {
		killall -9 dhclient &>/dev/null
		restart=1
	}

	# if no loop back interface is set up try to set 
	# one up if that fails exit.
	ifconfig lo &>/dev/null || \
	{
		ifconfig lo up 2>$stderr 1>$stdout 
		tc_fail_if_bad $? "no lo interface, and, fail to config lo" || exit
	}
}


#
# Function:    tc_local_cleanup
#
# Description: perform clean up, remove all temporary files 
#			   restart any deamons that were stopped.
#
tc_local_cleanup()
{
	killall dhclient &>/dev/null
	[ $restart -eq 1 ] && { dhclient &>/dev/null ; }
	rcnetwork restart
}


#
# Function:    test01
#
# Description:	- Test that 'dhclient -e interface' will look for DHCP server 
#		  on the interface and send DHCPDISCOVER messages. When
#		  no server is found it will complain that no DHCPOFFERS 
#		  received.
#	   	- execute command dhclient -e lo
#	   	- capture all messages to a file
#	   	- look for certain key words/phrases that will indicate
#		  expected behaviour.
#
# Inputs:	   NONE
#
# Exit		   0 - on success
#		   non-zero on failure.
test01()
{
	tc_register    "dhclient functionality"
	
	# execute dhclient command 'dhclient -e lo'
	tc_info "executing 'dhclient -e lo'"
	dhclient lo &>$TCTMP/tst_dhclient.out 
	[ $? -eq 0 ] && \
	{
		tc_fail_if_bad $? \
	"ghost DHCP server on lo\? $(cat $TCTMP/tst_dhclient.out)" || return ;
	}

	# check for key-words/phrases that will indicated expected 
	# behaviour.
	tc_info "check for key-words/phrases"
	grep -i "Listening on LPF/lo" $TCTMP/tst_dhclient.out \
	2>$stderr 1>$stdout && \
	grep -i "Sending on   LPF/lo/" $TCTMP/tst_dhclient.out \
	2>$stderr 1>$stdout && \
	grep -i "Sending on   Socket/fallback" \
	$TCTMP/tst_dhclient.out  2>$stderr 1>$stdout && \
	grep -i "DHCPDISCOVER on lo to" $TCTMP/tst_dhclient.out \
	2>$stderr 1>$stdout && \
	grep -i "No DHCPOFFERS received" $TCTMP/tst_dhclient.out \
	2>$stderr 1>$stdout && \
	grep -i "sleeping" $TCTMP/tst_dhclient.out \
	2>$stderr 1>$stdout
	tc_pass_or_fail $? "did not find phrases that indicate expeceted behaviour"
}


# Function: main
# 
# Description: - call setup function.
#		   - execute each test.
#
# Inputs:	   NONE
#
# Exit:		   zero - success
#		   non_zero - failure
#
TST_TOTAL=1
tc_setup
tc_exec_or_break $REQUIRED || exit
test01 
