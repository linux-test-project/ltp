#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab:
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
# File :	time
#
# Description:	Test the time command
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 06 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Jul 18 2003 - (rcp) separated out from "base" testcase.
#		16 Dec 2003 - (rcp) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register "is time installed?"
	tc_executes time
	tc_pass_or_fail $? "time not installed"
}

#
# test02	check out the time command
#
function test02()
{
	tc_register "time"
	tc_exec_or_break sleep grep || return

	# time sleep 5
	tc_info "timing \"sleep 5\""
	2>&1 time -p sleep 5 &>$stdout		# normal output is to stderr!
	cat $stdout | grep -q "^real *5"
	tc_pass_or_fail $? "expected to see approx 5 seconds in stdout"
}

################################################################################
# main
################################################################################

TST_TOTAL=2
tc_setup

test01 &&
test02
