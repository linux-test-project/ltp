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
# File :	sed.sh
#
# Description:	Test the stream editor (sed)
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
	tc_register "is sed installed?"
	tc_executes sed
	tc_pass_or_fail $? "sed not installed"
}

#
# test02	exercise sed a little
#
function test02()
{
	tc_register "sed"
	tc_exec_or_break cat chmod diff || return
	#
	# create source file
	cat > $TCTMP/sed_input <<-EOF
		line one
		line two
		line three
	EOF
	#
	# create sed script file
	local sed_cmd="`which sed`"
	cat > $TCTMP/sed_script <<-EOF
		#!$sed_cmd -f
		/two/s/line/LINE/
		/one/a\\
		first inserted line\\
		second inserted line
	EOF
	chmod +x $TCTMP/sed_script
	#
	# execute the sed command
	$TCTMP/sed_script $TCTMP/sed_input >$TCTMP/sed_output 2>$stderr
	tc_fail_if_bad $? "bad rc ($?) from sed" || return
	#
	# create file of expected results
	cat > $TCTMP/sed_exp <<-EOF
		line one
		first inserted line
		second inserted line
		LINE two
		line three
	EOF
	#
	# compare actual and expected results
	diff $TCTMP/sed_exp $TCTMP/sed_output >$TCTMP/sed_diff
	tc_pass_or_fail $? "actual and expected results do not compare" \
		"expected..."$'\n'"`cat $TCTMP/sed_exp`" \
		"actual..."$'\n'"`cat $TCTMP/sed_output`" \
		"difference..."$'\n'"`cat $TCTMP/sed_diff`"
}

################################################################################
# main
################################################################################

TST_TOTAL=2
tc_setup

test01 &&
test02
