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
# File :	rmdic.sh
#
# Description:	This testcase checks 'mvdic' command of canna.
#
# Author:	Shoji Sugiyama (shoji@jp.ibm.com)
#
# History:	Jun 04 2003 - Initial version created. 
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
# test01	
#
function test01()
{
	#
	# Register test case
	#
	tc_register "\"rmdic\" remove user dictionaries"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo mkdic rmdic lsdic || return
	
	#
	# Execute and check result
	#
	mkdic _testdic01 > /dev/null 2>&1
	rmdic _testdic01 > /dev/null 2>&1
	tc_pass_or_fail $? "[rmdic] failed to remove user dictionary."
}

#
# test02	
#
function test02()
{
	#
	# Register test case
	#
	tc_register "\"rmdic -G\" remove group dictionaries"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo mkdic rmdic lsdic || return
	
	#
	# Execute and check result
	#
	mkdic -G _testgdic01 > /dev/null 2>&1
	rmdic -G _testgdic01 > /dev/null 2>&1
	tc_pass_or_fail $? "[rmdic -G] failed to remove group dictionary."
}

#
# cleanup
#
function cleanup()
{
	#
	# Clear all test dict
	#
	for dic in `lsdic | fgrep _test`
	do
		rmdic $dic > /dev/null 2>&1
	done

	for dic in `lsdic -G | fgrep _test`
	do
		rmdic -G $dic > /dev/null 2>&1
	done
}

################################################################################
# main
################################################################################

TST_TOTAL=2

# standard setup
tc_setup

# tc_add_user_or_break    # in addition to standard setup, create a temporary user

cleanup
test01
test02
cleanup

# If you want a sequence of tests to each be dependent on the previous one
# having succeeded, chain them together with "&&" as follows ...
#
# test01 && \
# test02 && \
# test03 && \
# test04
