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
# File :	setserial.sh
#
# Description:	Test setserial -g /dev/ttyS0
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Mar 09 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		16 Dec 2003 - (rcp) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# test01	setserial -g /dev/ttyS0
#
function test01()
{
	tc_register "setserial -g"

	local device="/dev/ttyS0"
	tc_exist_or_break $device || return

	local command="setserial -g $device"
	$command >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected respones from $command"

	cat $stdout | grep "^$device.*Port.*IRQ" >/dev/null
	tc_pass_or_fail $? "Bad output from $command" \
		"expected to see \"$device\", \"Port\", and \"IRQ\" in stdout"
}

################################################################################
# main
################################################################################

TST_TOTAL=1

tc_setup

tc_root_or_break || exit

test01

