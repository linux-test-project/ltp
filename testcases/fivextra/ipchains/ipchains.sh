#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by	      ##
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
# File :	ipchains_tests.sh
#
# Description:	Test basic functionality of ipchains (firewall administration)
#		- Test #1:	ipchains -L will list all rules in the selected
#		- Test #2:	Test ipchains deny packets from perticular IP.
#				chain.
#
# Author:	Manoj Iyer, manjo@mail.utexas.edu
#
# History:	Feb 10 2003 - Created - Manoj Iyer.
#		Jun 20 2003 - Modified - check for the existance of commands
#					  used by this testcase.
#		Sep 18 2003 - Modified - using utility.sources
#		Sep 29 2003 - HACK - ping is broken, hangs on an ips that dont
#			  respond to ICMP messages. use -w option so that
#			  testcase will not hang.
#		08 Jan 2004 - (RR) updated to tc_utils.source, cleanup.
#		28 Jan 2004 (rcp) Improved error output.

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# tc_local_setup
#	 
# Description:	perform standard setup operations and also check if commands
#		used by this testcase are installed on the system.
#
tc_local_setup()
{

	# check dependencies
	tc_root_or_break || exit
	tc_exec_or_break diff cat wc || return

	# installed ipchains module.
	modprobe ipchains 2>$stderr 1>$stdout 
	tc_fail_if_bad $? "failed to load ipchains module" || return

	# check if ipchains works with this kernel version
	ipchains -L 2>$stderr 1>$stdout 
	tc_fail_if_bad $? "ipchains error"
}

#
# tc_local_cleanup
#
tc_local_cleanup()
{
	# remove all the deny rules.
	numrules=$(ipchains -L | grep "DENY" | wc -l)
	while [ "$numrules" -ne "0" ] ; do
		ipchains -D input 1
		numrules=$((numrules-1))
	done

	rmmod ipchains &>/dev/null
}

#
# test01	Installation check
#
function test01()
{
	tc_register "installation check"
	tc_executes ipchains
	tc_pass_or_fail $? "ipchains not installed propery"
}

#
# test02
#
# Description:	- Test basic functionality of ipchains (firewall administration)
#		- ipchains -L will list all rules in the selected chain.
#
function test02()
{
	tc_register    "ipchains setup"

	# create expected file
	cat >$TCTMP/tst_ipchains.exp <<-EOF
		Chain input (policy ACCEPT):
		Chain forward (policy ACCEPT):
		Chain output (policy ACCEPT):
	EOF

	# ipchains with list option, check if versions are compatible.
	ipchains -L >$stdout 2>$stderr
	tc_fail_if_bad $? "ipchains error" || return

	diff -qiwbE $stdout $TCTMP/tst_ipchains.exp >$TCTMP/diff.out 2>$stderr
	tc_pass_or_fail $? "expected output and actual output differ" \
			"difference: $(cat $TCTMP/diff.out)"
}

#
# test03
#
# Description:	- Test basic functionality of ipchains (firewall administration)
#		- Test ipchains deny packets from perticular IP.
#		- Append new rule to block all packets from loopback.
#		- ping -c 2 loopback, this should fail. 
#		- remove rule, and ping -c loopback, this should work.
#
function test03()
{
	tc_register "ipchains functionality"

	tc_info "Use ipchains to deny packets from perticular IP"
	tc_info "Rule to block icmp from 127.0.0.1"

	ipchains -A input -s 127.0.0.1 -p icmp -j DENY 2>$stderr 1>$stdout
	tc_fail_if_bad $? "ipchains command failed to append new rule." || return

	tc_info "Pinging 127.0.0.1"
	ping -w 10 -c 2 127.0.0.1 2>$stderr 1>$stdout
	grep -q "100.* loss" $stdout 2>>$stderr
	tc_fail_if_bad $? "ipchains did not block packets from loopback" || return

	tc_info "Deleting icmp DENY from 127.0.0.1 rule."
	ipchains -D input 1 2>$stderr 1>$stdout
	tc_fail_if_bad $? "ipchains did not remove the rule." || return

	tc_info "Pinging 127.0.0.1 again"
	ping -w 10 -c 2 127.0.0.1 2>$stderr 1>$stdout
	tc_pass_or_fail $? "ipchains still blocking loopback." 
}

# 
# main
# 

TST_TOTAL=3
tc_setup
test01 &&
test02 &&
test03
