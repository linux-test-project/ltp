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
# File :	tc_template.sh
#
# Description:	This is a template that can be used to develop shell script
#		testcases for the LTP. It tests "portmap" as a example.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 06 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Feb 10 2003 - Updates after code review.
#		Nov 06 2003 - (rcp) Updated for tc_utils.source

################################################################################
# Testcase Guidelines
################################################################################
#
# 1. A script file may contain multiple tests.
# 2. Every script MUST include the tc_utils.source file and issue the tc_setup
#    command before calling any of the other tc_* APIs.
# 3. Every test MUST start with tc_register.
# 4. Every test MUST provide an exit path that passes or fails (broken
#    paths are optional). This means that every test must have at least
#    one path that calls tc_pass_or_fail.
# 5. Once a test passes, fails, or breaks it MUST terminate. Other
#    test in the script can still run.
#
################################################################################
# Coding Guidelines
################################################################################
#
# Use a separate function for each test
#
# Use local variable within the functions.
#
# Functions should be short. If they get long or if the code gets deeply nested
# you should probably define some "helper functions".
#
# Use "helper functions" in preference to repeating nearly-identical code
# multiple times.
#
# Do not go past column 80.
#
# Tabs are 8 char wide. Use actual tabs (not blanks) for indentation. This way
# those who prefer narrower tabs can set their editor as they see fit, but be
# sure to stay within 80 chars when tab width is set to 8.
#
################################################################################

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

portmapinit="/etc/init.d/portmap"
must_stop_portmap="no"

################################################################################
# utility functions specific to this script
################################################################################

#
# tc_local_setup
#
#	This function is called automatically by the "tc_setup" function after
#	it has done the standard setup.
#
function tc_local_setup()
{
	tc_root_or_break || return	# this tc must be run as root
}

#
# tc_local_cleanup
#
#	This function is called automatically when your testcase exits
#	for any reason. It is called before standard cleanup is performed.
#
function tc_local_cleanup()
{
	[ $must_stop_portmap = "yes" ] && $portmapinit stop
}

################################################################################
# the test functions
################################################################################

#
# test01	check that portmap is installed
#
function test01()
{
	tc_register	"installation check"
	tc_executes $portmapcmd $portmapinit
	tc_pass_or_fail $? "portmap not installed"
}

#
# test02	start portmap if not already running
#
function test02()
{
	tc_register	"start portmap if not already running"

	# This is a little unusual in that there are two acceptable return
	# codes - 0 and 3. That makes the error checking a little more complex
	# than usual.
	$portmapinit status >$stdout 2>$stderr
	local rc=$?
	tc_break_if_bad $rc "bad break" || return
	if [ $rc -ne 0 ] && [ $rc -ne 3 ] ; then
		tc_fail_if_bad $rc "bad response from \"$portmapinit status\""
		return		# since we know the above failed (bad rc)
	fi

	# Note that even though there are two instances of tc_pass_or_fail
	# only one or the other can possibly be executed.
	if [ $rc -eq 3 ] ; then
		must_stop_portmap="yes"
		tc_info "starting portmap"
		$portmapinit start >$stdout 2>$stderr
		tc_pass_or_fail $? "could not start portmap"
	else
		tc_pass_or_fail 0 ""
		tc_info "(portmap was already running)"
	fi
}

#
# test03	check portmap info
#
#		This test is a good example of typical use of tc_executes
#		and tc_fail_if_bad in that both uses include "|| return" which
#		terminates the test upon early breakage or failure.
#
function test03()
{
	tc_register	"check portmap info"
	tc_executes grep || return

	local rpcinfocmd="rpcinfo -p"
	$rpcinfocmd >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from $rpcinfocmd" || return

	grep -q ".*tcp.*portmapper" $stdout 2>$stderr && \
	grep -q ".*udp.*portmapper" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see udp and tcp portmappers in stdout" \
			"command issued was $rpcinfocmd"
}

################################################################################
# main
################################################################################

TST_TOTAL=3	# there are three tests in this testcase

tc_setup	# standard setup

test01 &&
test02 &&
test03
