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
# File :	iptables.sh
#
# Description:	Test iptables support
#
# Author:	Helen Pang, hpang@us.ibm.com
#
# History:	Apr 14 2003 - Created. Helen Pang. hpang@us.ibm.com
#		May 07 2003 - Updates after code review
#		16 Dec 2003 - (hpang) updated to tc_utils.source
#		05 Feb 2004 - (rcp) general cleanup.
################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# any utility functions specific to this file can go here
################################################################################

function tc_local_setup()
{
	tc_exec_or_break iptables iptables-save iptables-restore || return
	tc_root_or_break || return

	# stat with a clean slate
	rmmod ipchains &>/dev/null
	rmmod iptable_mangle &>/dev/null
	rmmod ipt_state &>/dev/null
	rmmod ip_conntrack &>/dev/null
	rmmod iptable_filter &>/dev/null
	rmmod ip_tables &>/dev/null

	# force load of iptables module
	iptables -L &>/dev/null

	# save current rules, if any
	iptables-save -c >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables-save failed to save the current rule-set" \
		|| return
	cp $stdout $TCTMP/iptables-save

}

function tc_local_cleanup()
{
	# retore rules saved at beginning
	[ -s $TCTMP/iptables-save ] && {
		iptables-restore -c < $TCTMP/iptables-save >$stdout 2>$stderr
		tc_fail_if_bad $? "iptables_restore failed to restore rule-set" \
			|| return
	}
}

################################################################################
# the testcase functions
################################################################################

#
# test01   (set/check rules: append(-A), delete(-D), list(-L))
#
function test01()
{
	tc_register "use rule to block ping"

	# add DROP rule
	iptables -A INPUT -s 127.0.0.1 -p icmp -j DROP >$stdout 2>$stderr
	tc_fail_if_bad $? \
		"iptables -A INPUT -s 127.0.0.1 -p icmp -j DROP failed" ||
		return

	# be sure DROP rule is set
	iptables -L INPUT >$stdout
	tc_fail_if_bad $? "iptables -L INPUT failed" || return
	grep -q DROP $stdout 2>$stderr
	tc_fail_if_bad $? "failed to list DROP" || return

	# check that ping DROPped
	tc_info "Waiting 5 seconds for ping to timeout"
	ping -c 1 -w 5 127.0.0.1 &>$stdout
	[ $? -ne 0 ]
	tc_fail_if_bad $? "failed to DROP" || return

	# delete DROP rule
	iptables -D INPUT -s 127.0.0.1 -p icmp -j DROP >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -D INPUT -s 127.0.0.1 -p icmp -j DROP failed" || return

	# check that ping now woks
	tc_info "Ping now"
	ping -c 1 127.0.0.1 >$stdout 2>$stderr
	tc_pass_or_fail $? "failed to delete DROP; ping still woks!"
}

#
# test02   (set/check chain-command: create-chain(-N), and delete-chain(-X))
#
function test02()
{
	tc_register "set/check chain"
	tc_exec_or_break touch || return

	# add user defined new chain: -N 
	new="new-chain"
	iptables -N $new >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -N $new failed" || return

	# check if $new exist
	iptables -L >$stdout
	grep -q $new $stdout 2>$stderr
	tc_fail_if_bad $? "failed to create chain: $new" || return

	# remove user defined new chain: -X
	iptables -X $new >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -X $new failed" || return

	# $new should now be gone
	iptables -L >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -L failed" || return
	grep -q $new $stdout >> $stderr
	[ $? -ne 0 ]
	tc_pass_or_fail $? "failed to delete new chain"
}

#
# test03    (set/check policy for chains)
#
function test03()
{
        tc_register "set/check policy"

	# set/check policy on INPUT chain 
	iptables -P INPUT ACCEPT >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -P INPUT ACCEPT failed" || return
	iptables -L INPUT >$stdout
	tc_fail_if_bad $? "iptables -L INPUT failed" || return
	grep -q ACCEPT $stdout 2>$stderr
	tc_fail_if_bad $? "failed to set policy on INPUT" || return

	# set/check policy on OUTPUT chain
	iptables -P OUTPUT ACCEPT >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -P OUTPUT ACCEPT failed" || return
	iptables -L OUTPUT >$stdout
	tc_fail_if_bad $? "iptables -L OUTPUT failed" || return
	grep -q ACCEPT $stdout 2>$stderr
	tc_fail_if_bad $? "failed to set policy on OUTPUT" || return

	# set/check policy on FORWARD chain
	iptables -P FORWARD ACCEPT >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -P FORWARD ACCEPT failed" || return
	iptables -L FORWARD >$stdout
	tc_fail_if_bad $? "iptables -L FORWARD failed" || return
	grep -q ACCEPT $stdout 2>$stderr
	tc_pass_or_fail $? "failed to set policy on FORWARD"
}	

#
# test04    (set/check match -m  and state --state)
#
function test04()
{
	tc_register "set/check match and state"

	# set -m with its --state options for FORWARD chain
	iptables -A FORWARD -m state --state NEW -j ACCEPT >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -A FORWARD -m state --state NEW -j ACCEPT failed" || return

	# check -m and --state
	iptables -L FORWARD >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -L FORWARD failed" || return
	grep -q NEW $stdout 2>$stderr
	tc_pass_or_fail $? "failed to set match and state options"
}

#
# test05    (set/check flush in filter and mangle tables)
#
function test05
{
	tc_register "set/check flush"

	# flush the filter table
	iptables -F >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -F failed" || return

	# add DROP so we can flush it
	iptables -A INPUT -j DROP >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -A INPUT -j DROP failed" || return
	iptables -L >$stdout
	tc_fail_if_bad $? "iptables -L failed" || return
	grep -q DROP $stdout 2>$stderr
	tc_fail_if_bad $? "failed to set DROP" || return

	# flush the filter table and be sure DROP is gone
	iptables -F >$stderr 2>$stdout
	tc_fail_if_bad $? "iptables -F failed" || return
	iptables -L >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -L failed" || return
	grep -q DROP $stdout 2>$stderr
	[ $? -ne 0 ]
	tc_fail_if_bad $? "failed to flush DROP" || return

	# flush the mangle table
	iptables -F -t mangle >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -F -t mangle failed" || return

	# add DROP so we can flush it
	iptables -t mangle -A INPUT -j DROP >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -t mangle -A INPUT -j DROP failed" || return
	iptables -L -t mangle >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -L -t mangle failed" || return
	grep -q DROP $stdout 2>$stderr
	tc_fail_if_bad $? "failed to set DROP" || return

	# flush the mangle table and be sure DROP is gone
	iptables -F -t mangle >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -F -t mangle failed" || return
	iptables -L -t mangle >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables -L -t mangle failed" || return
	grep -q DROP $stdout 2>$stderr
	[ $? -ne 0 ]
	tc_pass_or_fail $? "failed to flush DROP"
}	

##########################################################################################
# main
##########################################################################################

TST_TOTAL=5

# standard tc_setup
tc_setup

test01 && 
test02 && 
test03 && 
test04 && 
test05  
