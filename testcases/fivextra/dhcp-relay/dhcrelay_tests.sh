#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
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
# File:		dhcrelay_tests.sh
#
# Description:	This program tests basic functionality of dhcrelay program
#
# Author:	Manoj Iyer  manjo@mail.utexas.edu
#
# History:	Sept 02 2003 - created - Manoj Iyer
#		Oct 06 2003 - Modified - Manoj Iyer
#			change dhcp-relay in set up to dhcrelay.
#			look for send and listen messages only.
#		08 Jan 2004 - (RR) updated to tc_utils.source, cleanup.
#		11 Feb 2004 (rcp) determine interface dynamically;
#			don't assume eth0

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

restart=0	   # flag to indicate if dhcp needs to be restarted on exit

#
# tc_local_setup
#	 
function tc_local_setup()
{	
	# check dependencies
	tc_root_or_break || exit
	tc_exec_or_break ps dhcrelay diff tail cat || exit

	# stop any dhcprelay if already running.
	ps -C dhcrelay | grep dhcrelay &>/dev/null && \
	{  
		/etc/init.d/dhcrelay stop ||
			/etc/init.d/dhcp-relay stop
		tc_fail_if_bad $? "failed to stop dhcp-relay" || return
		restart=1 ;
	} 

	#find interface to use
	set $(route | grep default)
	iface=$8
	[ "$iface" ]
	tc_break_if_bad $? "can't find network interface" || return
}

#
# tc_local_cleanup
#
tc_local_cleanup()
{
	killall -9 dhcrelay &>/dev/null
	[ $restart -eq 1 ] && { /etc/init.d/dhcp-relay start &>/dev/null ; }
}

#
# test01	installation check
#
function test01()
{
	tc_register	"dhcrelay installation check"
	tc_executes dhcrelay
	tc_pass_or_fail $? "dhcrelay not instaled properly"
}

#
# test02	Test that 'dhcrelay server_name' will listen for DHCP request 
#		on the interface.
#
#		Execute command dhcrelay on 127.0.0.1 and check if dhcrelay is 
#		listening.
#
function test02()
{
	tc_register    "dhcrelay functionality"

	tc_info "executing dhcrelay 127.0.0.1"
	dhcrelay -i $iface 127.0.0.1 &>$TCTMP/tst_dhcrelay.out.1
	tc_fail_if_bad $? "dhcrelay failed to start $(cat $TCTMP/tst_dhcrelay.out.1)"

	tail -n3 $TCTMP/tst_dhcrelay.out.1 | head -n 2 &> $TCTMP/tst_dhcrelay.out

	# create the expected output
	set $(ifconfig $iface | head -n1)
	local mac=$5
	cat <<-EOF >$TCTMP/tst_dhcrelay.exp 
		Listening on LPF/$iface/$mac
		Sending on LPF/$iface/$mac
	EOF

	diff -qiwB $TCTMP/tst_dhcrelay.exp $TCTMP/tst_dhcrelay.out 2>$stderr 
	tc_pass_or_fail $? "failed to produce expected output."
}

# 
# main
# 
TST_TOTAL=2
tc_setup
test01 &&
test02
