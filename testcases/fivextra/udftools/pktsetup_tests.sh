#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2001                                               ##
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
# File :	   pktcddvd_tests.sh
#
# Description: This program tests basic functionality of pktsetup command
#
# Author:	   Manoj Iyer  manjo@mail.utexas.edu
#
# History:	July 30 2003 - created - Manoj Iyer
#		Oct 07 2003 - Fixed - Robert Paulsen.
#		- checking for wrong command, and improve grep for messages
#		Oct 21 2003 (rcp) BUG 4892
#		- It is only BROK, not FAIL if /dev/pktcdvd is missing
#		- Added installationj check
#		Oct 24 2003 - Fixed - Paul Washington
#		- Changed device name from 'pktcdvd' to 'pktcdvd0' on exists command
#		08 Jan 2004 - (RR) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# test01	installation check
#
function test01()
{
	tc_register	"installation check"
	tc_executes pktsetup
	tc_pass_or_fail $? "not installed properly"
}

#
# test02	pktsetup functionality
#
function test02()
{
	tc_register    "pktsetup functionality"

	tc_exist_or_break /dev/pktcdvd0  || return

	pktsetup /dev/pktcdvd /dev/null &>$stdout
	# this line to be uncommented for real h/w write test.
	# tc_fail_if_bad $? "failed writing file to media."

	# check if certain messages appear as a result of this command.
	local exp1="drive not ready"
	local exp2="Inappropriate ioctl for device"
	grep -iq "$exp1" $stdout 2>$stderr || grep -q "$exp2" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected  to see" "$exp1" "$exp2" "in stdout"
}

#
# main
# 

TST_TOTAL=2
tc_setup
tc_root_or_break || exit
tc_exec_or_break  cat grep pktsetup || exit

test01 &&
test02
