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
# File :	procinfo.sh
#
# Description:	Test that procinfo is installed and working
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	02 Mar 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		03 Jan 2004 - (rcp) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# test01	check that procinfo is installed 
#
#		We consider it a failure if procinfo isn't installed.
#		Test case shouldn't be run if procinfo not installed.
#
function test01()
{
	tc_register "is procinfo installed?"
	which procinfo >/dev/null
	tc_pass_or_fail $? "procinfo not installed"
}

#
# test02	check that procinfo returns Linux release number
#
function test02()
{
	tc_register "get info from procinfo"
	tc_exec_or_break grep || return
	tc_exist_or_break /proc/sys/kernel/osrelease || return
	local osrelease="`cat /proc/sys/kernel/osrelease`"
	procinfo >$stdout 2>$stderr
	cat $stdout | grep -q "$osrelease"
	tc_pass_or_fail $? "expected to see \"$osrelease\" in stdout"
}

#
# test03	check that procinfo -m returns device data
#
function test03()
{
	tc_register "get device data from procinfo"
	tc_exec_or_break grep || return
	local procinfo="`procinfo -m 2>$stderr`"
	procinfo -m >$stdout 2>$stderr
	cat $stdout | grep -q "Character Devices.*Block Devices"
	tc_pass_or_fail $? "No Device data in stdout"
}

################################################################################
# main
################################################################################

TST_TOTAL=3

tc_setup			# standard setup

if test01 ; then
	test02 
	test03
fi
