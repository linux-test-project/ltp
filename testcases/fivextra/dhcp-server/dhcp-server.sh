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
# File :        dhcpcd.sh
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#		Based on ideas from Manoj Iyer.
#
# Description:  Test dhcp server daemon.
#
# History:      05 Mar 2003 - Created - Robert Paulsen, rpaulsen@us.ibm.com 
#		15 Dec 2003 (rcp) updated to tc_utils.source
#		11 Feb 2004 (rcp) find interface dynamically; don't assume eth0
#		17 Feb 2004 (rcp) fixed test for already-running dhcp-server.

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variabes
dhcpd_server_started="no"
mac_addr=""
subnet="10.1.1"

declare -i acnt=0	# count aliases
declare -a aliasips	# aliased ip addresses
declare -a aliasdevs	# matching aliased devices
aliasip=""		# most recently created alias ip
aliasdev=""		# most recently created alias interface

################################################################################
# local utility functions
################################################################################

#
# Alias $dev for private network.
#
#	$1	alias number to use
#	$2	ip address to use
#
function alias()
{
	# remember to unalias in cleanup
	let acnt+=1
	local myif=$dev:$1;	aliasdevs[acnt]=$myif;	aliasdev=$myif
	local myip=$2;		aliasips[acnt]=$myip;	aliasip=$myip
	alias=$myip

	ifconfig $myif inet $myip
	tc_break_if_bad $? "failed to alias $myif as $myip" || return
	route add -host $myip dev $myif
	tc_break_if_bad $? "failed to add route for $myip" || return
}

# Don't run this test if a local dhcpd server is already running.
# Breaks testcase and returns false if it is not safe to run the test.
# Returns true if it is safe to run the test.
function ok_to_run_dhcpcd()
{
	ps -e | grep -q dhcpd
	[ $? -ne 0 ]
	tc_break_if_bad $? "Can't run this test: dhcpd server is already running"
}

#
# tc_local_setup specific to this testcase.
#
function tc_local_setup()
{
	tc_root_or_break || return
        tc_exec_or_break ifconfig route || return
	acnt=0
	ok_to_run_dhcpcd || return # don't run this test if active dhcpd server
	set $(route | grep default)
	dev=$8
	[ "$dev" ]
	tc_break_if_bad $? "can't find network interface" || return
}

# Cleanup specific to this testcase.
function tc_local_cleanup()
{
	# stop our dhcpd server
	if [ "$dhcpd_server_started" = "yes" ] ; then
		killall dhcpd
		tc_info \
			"stopped the dhcpd server started by this testcase"
	fi

	# remove aliases
	if tc_executes ifconfig ; then
		while [ $acnt -gt 0 ] ; do
			local myip=${aliasips[acnt]}
			local myif=${aliasdevs[acnt]}
			ifconfig $myif inet $myip down &>/dev/null
			let acnt-=1
		done
	fi
}

################################################################################
# the testcase functions
################################################################################

#
#	Ensure dhcpcd package is installed
#
function test01()
{
	tc_register "is dhcpcd installed?"
	[ -x /etc/init.d/dhcpd ] 
	tc_pass_or_fail $? "dhcpcd package is not installed"
}

#
#	Start the dhcp server
#
function test02()
{
	tc_register "start dhcpd server"
	tc_exec_or_break cat grep || return

	# set up alias network
	alias 1 $subnet.12 || return

	# create dhcpd.conf file for this test
	cat > $TCTMP/dhcpd.conf <<-EOF
		ddns-update-style none;
		subnet $subnet.0 netmask 255.255.255.0 {
			range $subnet.13 $subnet.13;
			default-lease-time 600;
			max-lease-time 1200;
			option routers $subnet.1;
			option subnet-mask 255.255.255.0;
			option domain-name-servers $subnet.1;
			option domain-name "dhcptest.net";
		}
	EOF

	# start the dhcpd server
	dhcpd_server_started="yes"
	dhcpd -cf $TCTMP/dhcpd.conf &>$stdout	# dhcpd stupidly puts normal
						# output in stderr.
	tc_fail_if_bad $? "bad response from dhcpd"

	cat $stdout | grep -qi "Listening.*$subnet.*24" && \
	cat $stdout | grep -qi "Sending.*$subnet.*24"
	tc_pass_or_fail $? "cannot start dhcpd server"
}

################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup				# standard setup

test01 &&
test02
