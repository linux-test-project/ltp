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
# File:		dhcping_tests.sh
#
# Description:	This program tests basic functionality of dhcping program
#
# Author:	Manoj Iyer  manjo@mail.utexas.edu
#
# History:	Sept 02 2003 - created - Manoj Iyer
#		08 Jan 2004 - (RR) updated to tc_utils.source, cleanup
#		11 Feb 2004 (rcp) find interface dynamically; don't assume eth0
#		17 Feb 2004 (rcp) Add installation check. Clean up final check.

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

ipaddr=0	  # ip address of the system
hwaddr=0	  # hw address of this system

#
# tc_local_setup
#
function tc_local_setup()
{	
	# check dependencies
	tc_root_or_break || return
	tc_exec_or_break grep awk tail || return

	# find na interface
	set $(route | grep default)
	iface=$8
	[ "$iface" ]
	tc_break_if_bad $? "Could not find network interface" || return

	# get ip-address and hw-address.
	ipaddr=$(ifconfig $iface | head -n2 | tail -n1 | awk '{print $2}' | cut -f2 -d:)
	hwaddr=$(ifconfig $iface | head -n1| awk '{print $5}')

	tc_info "ipaddress: $ipaddr; hwaddress: $hwaddr; interface: $iface"
}

#
# test00	Installation check
#
function test00()
{
	tc_register	"dhcping installation check"
	tc_executes dhcping
	tc_pass_or_fail $? "dhcping not installed properly"
}

#
# test01	Test that dhcping will send a DHCP request to DHCP server 
#		to see if it's up and running.
#
#		execute the command dhcping with parametes and check for 
#		expected result. There is no dhcp server listening on 
#		127.0.0.1 so we should not get any replies for this qyery.
#
function test01()
{
	tc_register    "dhcping functionality"

	tc_info "executing dhcping -c $ipaddr -s 127.0.0.1 -h $hwaddr"
	2>&1 dhcping -c $ipaddr -s 127.0.0.1 -h $hwaddr | grep -q "no answer"
	tc_pass_or_fail $? "dhcping seems to receive answer from DHCP server on 127.0.0.1"
}

#
# main
# 
TST_TOTAL=2
tc_setup
test00 &&
test01 
