#!/bin/bash
################################################################################
#                                                                              #
#  (C) Copyright IBM Corp. 2003                                                #
#                                                                              #
#  This program is free software;  you can redistribute it and/or modify       #
#  it under the terms of the GNU General Public License as published by        #
#  the Free Software Foundation; either version 2 of the License, or           #
#  (at your option) any later version.                                         #
#                                                                              #
#  This program is distributed in the hope that it will be useful, but         #
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY  #
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    #
#  for more details.                                                           #
#                                                                              #
#  You should have received a copy of the GNU General Public License           #
#  along with this program;  if not, write to the Free Software                #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA     #
#                                                                              #
################################################################################
#
# File :       cdrwtool_tests.sh
#
# Description: This program tests basic functionality of cdrwtool command
#
# Author:      Manoj Iyer  manjo@mail.utexas.edu
#
# History:     July 30 2003 - created - Manoj Iyer
#		Oct 07 2003 - RC Paulsen: General cleanup
#		08 Jan 2004 - (RR) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# tescase functions
################################################################################

#
#	test01	installation check
#
function test01()
{
	tc_register "check installation"
	tc_executes cdrwtool
	tc_pass_or_fail $? "cdrwtool not installed properly"
}

#
#	test02	Test the functionality of cdrwtool command
#
function test02()
{
	tc_register    "cdrwtool functionality"
	tc_exist_or_break /dev/cdrom || return

	# create a dummy file to write
	cat <<-EOF > $TCTMP/tst_cdrwtool.in
	This is a dummy file to test cdrwtool
	aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
	bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
	ccccccccccccccccccccccccccccccccccccc
	ddddddddddddddddddddddddddddddddddddd
	eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
	EOF

	# use cdrwtool command to write this file 
	cdrwtool -d /dev/cdrom -f $TCTMP/tst_cdrwtool.in >$stdout 2>/dev/null

	# next line to be uncommented for real h/w write test.
	# tc_fail_if_bad $? "failed writing file to media." || return

	# check if certain messages appear as a result of this command.
	local exp1="using device /dev/cdrom"
	local exp2="write file $TCTMP/tst_cdrwtool.in"
	grep -q "$exp1" $stdout && grep -q "$exp2" $stdout 2>$stderr
	tc_pass_or_fail $? "expected to see" "$exp1" "$exp2" "in stdout"
}

################################################################################
# Main
################################################################################

TST_TOTAL=2
tc_setup        # exits on failure
tc_root_or_break || exit
tc_exec_or_break  cat grep || exit

test01 &&
test02
