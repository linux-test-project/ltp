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
# File :	ifuser.sh
#
# Description:	Test the ifuser program from the sysconfig package.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Sep 04 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Oct 27 2003 (rcp) Minor cleanup.
#		16 Dec 2003 - (robert) updated to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

my_ip=""	# set by tc_local_setup
my_if=""	# set by tc_local_setup

################################################################################
# any utility functions specific to this file can go here
################################################################################

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_exec_or_break /sbin/ip || return

	# should set	$3 to router's IP
	#		$5 to interface (e.g. eth0)
	/sbin/ip route > $stdout 2>$stderr
	tc_break_if_bad $? "Unexpected response from \"/sbin/ip route\"" || return

	set `grep "default via" $stdout`; my_ip=$3; my_if=$5
	[ "$my_ip" ] && [ "$my_if" ]
	tc_break_if_bad $? "Networking not set up properly" \
			"Unexpected stdout from \"/sbin/ip route\"" \
			"stdout shows ip=$my_ip and interface=$my_if" \
			|| return
}

################################################################################
# the testcase functions
################################################################################

#
# test01	ifuser installed
#
function test01()
{
	tc_register	"ifuser installed?"
	tc_executes /sbin/ifuser
	tc_pass_or_fail $? "ifuser not installed"
}

#
# test02	ifuser functionality
#
function test02()
{
	tc_register	"ifuser functionality"

	/sbin/ifuser -v $my_if $my_ip >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from /sbin/ifuser -v $my_if $my_ip" || return
	grep -q "$my_if" $stdout
	tc_fail_if_bad $? "Did not see $my_if in stdout" || return
	grep -q "$my_ip" $stdout
	tc_pass_or_fail $? "Did not see $my_ip in stdout"
}

################################################################################
# main
################################################################################

TST_TOTAL=2

# standard tc_setup
tc_setup

test01 &&
test02
