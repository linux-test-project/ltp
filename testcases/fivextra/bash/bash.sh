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
# File :	bash.sh
#
# Description:	Check that the bash shell can run
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Mar 09 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		15 Dec 2003 (rcp) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

function test01()
{
	tc_register "check that bash can execute"
	#
	tc_exec_or_break cat chmod || return
	#
	local expected="running in bash shell"
	cat > $TCTMP/trybash <<-EOF
		#!/bin/bash
		echo "$expected"
		exit 0
	EOF
	chmod +x $TCTMP/trybash
	#
	$TCTMP/trybash >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response" || return
	#
	cat $stdout | grep -q "$expected" 2>$stderr
	tc_pass_or_fail $? "Did not see expected output \"$expected\" in stdout"
}

################################################################################
# main
################################################################################

TST_TOTAL=1
tc_setup			# standard tc_setup

test01
