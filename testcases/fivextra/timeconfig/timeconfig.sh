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
# File :	timeconfig.sh
#
# Description:	Testcase for the timeconfig program.
#
# Author:	Amos Waterland <apw@us.ibm.com>
#
# History:	Mar 20 2003 - Created.
#		05 Jan 2004 - (rcp) updated to tc_utils.source


################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# any utility functions specific to this file can go here
################################################################################

################################################################################
# the testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register "installation checK"

	tc_executes  timeconfig
	tc_pass_or_fail $? "timeconfig not properly installed"
}

#
# test02	timeconfig (check that graphical support is disabled)
#
function test02()
{
	tc_register "\"timeconfig\" check that graphical support off"
#
	timeconfig >$stdout 2>$TCTMP/output
	rc=$?
	[ $rc -eq 2 ]
	tc_pass_or_fail $? "unexpected return value" \
		"Expected retval: 2" \
		"            Got: $rc"
}

#
# test03	timeconfig - (verify that data file exists)
#
function test03()
{
	tc_register "\"test -f /etc/sysconfig/clock\" verify data file"
#
	test -f /etc/sysconfig/clock >$stdout 2>$stderr
	tc_pass_or_fail $? "file does not exist"
}

#
# test04	timeconfig - (set timezone to UTC)
#
function test04()
{
	tc_register "\"timeconfig UTC\" set timezone"
#
	timeconfig UTC >$stdout 2>$stderr
	tc_fail_if_bad $? "timeconfig returned $?" || return

	test -f /etc/localtime
	tc_fail_if_bad $? "/etc/localtime was not created" || return

	test -f /etc/sysconfig/clock
	tc_pass_or_fail $? "/etc/sysconfig/clock was not created"
}

#
# test05	timeconfig - (set timezone to US/Central)
#
function test05()
{
	tc_register "\"timeconfig US/Central\" set timezone"
#
	timeconfig US/Central >$stdout 2>$stderr
	tc_fail_if_bad $? "timeconfig returned $?" || return

	test -f /etc/localtime
	tc_fail_if_bad $? "/etc/localtime was not created" || return

	test -f /etc/sysconfig/clock
	tc_pass_or_fail $? "/etc/sysconfig/clock was not created"
}

################################################################################
# main
################################################################################

# standard setup
tc_setup

TST_TOTAL=5

tc_root_or_break || exit

test01 || exit
test02
test03
test04
test05
