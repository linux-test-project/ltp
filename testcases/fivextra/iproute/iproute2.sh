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
# File:		iproute2.sh
#
# Description:	Test basic functionality of ip command in iproute2 package
#
# Author:	Manoj Iyer, manjo@mail.utexas.edu
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 19 2003 - Created - Manoj Iyer
#               Feb 26 2003 - Added - test05, test06
#                           - Commands mroute, tunnel, monitor and rtmon are
#                             not covered by this testcase.
#		Mar 30 2003 - (rcp)
#			- No longer assumes added neighbor is at top of list.
#			- Now restores MTU.
#			- Now unloads dummy module.
#			- Now waits 30 sec for ip neigh del to complete
#			  asynchronously.
#			- Simplified nested if-else, etc.
#			- Converted to use standard utilities.
#		Oct 20 2003 (rcp)
#			- Fix BUG 4816:
#			- MTU field may not be field # 6 -- use more forgiving
#				technique to extract MTU from output.
#			- Initialize some variables for cleaner error path.
#			- Use new tc_break_if_bad function instead of
#				tst_brokm TBROK.
#			- Use new tc_local_setup and tc_local_cleanup functions.
#			- general cleanup.
#		03 Jan 2004 - (rcp) updated to tc_utils.source
#		08 Jan 2004 - (RR) minor cleanup.

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# globals
################################################################################

declare -i acnt=0	# count aliases
declare -a aliasips=""	# aliased ip addresses
declare -a aliasdevs=""	# matching aliased devices
aliasip=""		# most recently created alias ip
aliasdev=""		# most recently created alias interface
dev="eth0"
myloop="127.6.6.6"
orig_mtu=""
rmmod_dummy=""

################################################################################
# any utility functions specific to this file can go here
################################################################################

#
# Alias $dev for private network.
#
#	$1	alias number to use
#	$2	ip address to use
#
function alias()
{
        tc_exec_or_break ifconfig || return

	# remember to unalias in cleanup
	let acnt+=1
	local myif=$dev:$1;	aliasdevs[acnt]=$myif;	aliasdev=$myif
	local myip=$2;		aliasips[acnt]=$myip;	aliasip=$myip

        ifconfig $myif inet $myip || echo BAD!
	tc_break_if_bad $? "failed to alias $myif as $myip" || return

	route add -host $myip dev $myif
	tc_break_if_bad $? "failed to add route for $myip" || return

        return 0
}

#
# Setup specific to this testcase.
#
function tc_local_setup()
{
	tc_root_or_break || return
	tc_exec_or_break ifconfig route || return
}

#
# Cleanup specific to this testcase.
#
function tc_local_cleanup()
{
	# remove aliases
	while [ $acnt -gt 0 ] ; do
		local myip=${aliasips[acnt]}
		local myif=${aliasdevs[acnt]}
		ifconfig $myif inet $myip down &>/dev/null
		let acnt-=1
	done

	# restore original MTU
	type -p ip >/dev/null && [ "$orig_mtu" ] && ip link set $dev mtu $orig_mtu

	# remove dummy module if we loaded it
	[ "$rmmod_dummy" = "yes" ] && rmmod dummy &>/dev/null
}

################################################################################
# the testcase functions
################################################################################

################################################################################
#
#	test01		See that iproute2 packge is installed (or at least that
#			the ip command is available).
#
function test01()
{
	tc_register	"installed"
	tc_executes ip
	tc_pass_or_fail $? "iproute2 package not properly installed"
}

################################################################################
#
#	test02		See that "ip link set DEVICE mtu MTU"
#			changes the device mtu.
#
function test02()
{
	tc_register	"ip link set"

	# save original MTU to be restored later
	local mtu_line=`ifconfig $dev | grep MTU`
	mtu_line=${mtu_line##*MTU:}
	orig_mtu=${mtu_line%% *}

	alias $TST_COUNT 10.1.1.12 || return
	local command="ip link set $aliasdev mtu 300"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	ifconfig $aliasdev >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from ifconfig $aliasdev" || return

	grep -q "MTU:300" $stdout 2>$stderr
	tc_pass_or_fail $? "MTU not set to 300"
}

################################################################################
#
#	test03		See that "ip link show" lists device attributes.
#
function test03()
{
	tc_register	"ip link show"
	tc_exec_or_break lsmod modprobe grep || return

	# remember to rmmod the dummy module
	if ! lsmod | grep "^dummy *" >/dev/null ; then
		rmmod_dummy="yes"
	fi

	modprobe dummy >$stdout 2>$stderr 
	tc_break_if_bad $? "failed to load module \"dummy\"" || return

	local command="ip link show dummy0"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	grep -q dummy0 $stdout 2>$stderr
	tc_pass_or_fail $? "\"$command\" failed to show dummy0 attributes"
}

################################################################################
#
#	test04		See that "ip addr add <ip address> dev <device>"
#			will add new protocol address.
#
function test04()
{
	tc_register	"ip addr add"
	tc_exec_or_break grep || return

	local command="ip addr add $myloop dev lo"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	ip addr show dev lo >$stdout 2>$stderr
	grep -q "$myloop" $stdout 2>$stderr
	tc_pass_or_fail $? "\"$command\" did not add protocol address"
}

################################################################################
#
#	test05		See that "ip addr del <ip address> dev <device>"
#			will delete the protocol address added in test03.
#
function test05()
{
	tc_register	"ip addr del"
	tc_exec_or_break grep || return

	local command="ip addr del $myloop dev lo"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	command="ip addr show dev lo"
	$command 2>$stderr >$stdout
	grep -vq "$myloop" $stdout 2>$stderr
	tc_pass_or_fail $? "\"$command\" did not delete protocol address $myloop"
}

################################################################################
#
#	test06		See that "ip neigh add" adds new neighbor entry
#			to arp table.
#
function test06()
{
	tc_register	"ip neigh add"
	tc_exec_or_break grep || return

	local loopb="127.0.0.1"
	local command="ip neigh add $loopb dev lo nud reachable"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	local exp="$loopb dev lo lladdr 00:00:00:00:00:00 nud reachable"
	local command="ip neigh show"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	grep -q "$exp" $stdout 2>$stderr
	tc_pass_or_fail $? "\"$command\" did not add neighbor" \
		"expected to see"$'\n'"$exp in stdout"
}

################################################################################
#
#	test07		See that "ip neigh del" deletes the new neighbor
#			added in test06.
#
function test07()
{
	tc_register	"ip neigh del"

	local loopb="127.0.0.1"
	local command="ip neigh del $loopb dev lo"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	tc_info "waiting 5 secs for ip command to finish asynchronously"
	sleep 5

	command="ip neigh show"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	grep -vq "$loopb" $stdout 2>$stderr
	tc_pass_or_fail $? "$loopb still listed in arp."
}

################################################################################
#
#	test08		See that "ip maddr add" adds a multicast addr entry
#
function test08()
{
	tc_register	"ip maddr add"

	alias $TST_COUNT 10.6.6.6 || return
	local command="ip maddr add 66:66:00:00:00:66 dev $aliasdev"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	command="ip maddr show"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	grep "link" $stdout | grep "66:66:00:00:00:66" | grep -q "static" 2>$stderr
	tc_pass_or_fail $? "unexpected result from \"$command\"" \
		"expected to see"$'\n'"$exp in stdout"
	
}

################################################################################
#
#	test09		See that "ip maddr del" deletes the multicast addr entry
#			created in test08.
#
function test09()
{
	tc_register	"ip maddr del"

	local hwaddr="66:66:00:00:00:66"
	local command="ip maddr del $hwaddr dev $aliasdev"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	command="ip maddr show"
	$command 2>$stderr >$stdout
	tc_fail_if_bad $? "unexpected result from \"$command\"" || return

	grep -q "$hwaddr" $stdout 2>$stderr >$TCTMP/output
	[ ! -s $TCTMP/output ]
	tc_pass_or_fail $? "unexpected result from \"$command\"" \
		"expected to NOT see"$'\n'"$hwaddr in stdout"
}

################################################################################
# main
################################################################################

TST_TOTAL=9

tc_setup

test01 || exit
test02
test03 
test04
test05 
test06
test07
test08
test09

