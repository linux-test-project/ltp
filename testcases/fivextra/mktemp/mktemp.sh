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
# File :	mktemp.sh
#
# Description:	Utility for creating a temporary unique file name 
#
# Author:	Helen Pang, hpang@us.ibm.com
#
# History:	Sept. 16 2003 - Created. Helen Pang. hpang@us.ibm.com
#
#		16 Dec 2003 - (hpang) updated to tc_utils.source
################################################################################
# source the standard utility functions
###############################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# test01       is utility mktemp installed 
#				   
function test01()
{
        tc_register "is utility mktemp installed"

        tc_executes mktemp
	tc_pass_or_fail $? "Failed installing the mktemp"
}

#
# test02	create a temporary file
#
function test02()
{
	tc_register "create a temporary file"
	tc_exec_or_break cat || return
	
	mktemp $TCTMP/f.XXXXXX >$stdout 2>$stderr
	tc_fail_if_bad $? "Failed creating temp file" || return
	[ -f "`cat $stdout`" ]
	tc_pass_or_fail $? "failed to create temp file or failed to report correct file name"
}

#
# test03	check -q with error
#
function test03()
{
	tc_register "fail with -q"
	tc_exec_or_break cat || return
	
	# force error - not enough X's
	mktemp -q $TCTMP/fq.XXXXX >$stdout 2>$stderr
	[ $? != 0 ]
	tc_fail_if_bad $? "Failed the failing silently option -q for mktemp" || return
	[ ! -f "`cat $stdout`" ]
	tc_pass_or_fail $? "failed to create temp file or failed to report correct file name"
}

#
# test04        create a temporary directory
#
function test04()
{
	tc_register "create a temporary directory"
	tc_exec_or_break cat || return

	mktemp -dq $TCTMP/dq.XXXXXX >$stdout 2>$stderr
	tc_fail_if_bad $? "Failed making a temporary directory" || return
	[ -d "`cat $stdout`" ]
	tc_pass_or_fail $? "failed to create temp directory or failed to report correct directory name"
}

################################################################################
# main
################################################################################

TST_TOTAL=4

# standard tc_setup
tc_setup

test01 &&
test02 &&
test03 &&
test04 
