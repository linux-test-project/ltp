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
# File :	cpdic.sh
#
# Description:	This testcase checks 'cpdic' command of canna.
#
# Author:	Shoji Sugiyama (shoji@jp.ibm.com)
#
# History:	Jun 02 2003 - Initial version created. 
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
	tc_register "\"cpdic\" copy user dictionaries"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo mkdic rmdic lsdic cpdic || return
	
	#
	# Execute and check result
	#
	mkdic _testdic01 > /dev/null 2>&1
	cpdic _testdic01 _testdic02 > /dev/null 2>&1
	tc_fail_if_bad $? "[cpdic] failed to copy user dictionary." || return

	num=`lsdic | grep -E "(_testdic01)|(_testdic02)" | wc -l`
	[ $num = 2 ]
	tc_pass_or_fail $? "[cpdic] failed to find copied dictionary."
}

#
# test02	
#
function test02()
{
	#
	# Register test case
	#
	tc_register "\"cpdic -G\" copy group dictionaries"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo mkdic rmdic lsdic cpdic || return
	
	#
	# Execute and check result
	#
	mkdic -G _testgdic01 > /dev/null 2>&1
	cpdic -G _testgdic01 _testgdic02 > /dev/null 2>&1
	tc_fail_if_bad $? "[cpdic -G] failed to copy group dictionary." || return

	num=`lsdic -G | grep -E "(_testgdic01)|(_testgdic02)" | wc -l`
	[ $num = 2 ]
	tc_pass_or_fail $? "[cpdic -G] failed to find copied group dictionary."
}

#
# test03	
#
function test03()
{
	#
	# Register test case
	#
	tc_register "\"cpdic -i\" copy system dictionary"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo mkdic rmdic lsdic cpdic || return
	
	cleanup
	#
	# Execute and check result
	#
	cpdic -i bushu _testdic01 > /dev/null 2>&1
	tc_fail_if_bad $? "[cpdic -i] failed to copy system dictionary." || return

	num=`lsdic | grep _testdic01 | wc -l`
	[ $num = 1 ]
	tc_pass_or_fail $? "[cpdic -i] failed to find copyied system dictionary."
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

TST_TOTAL=3

# standard setup
tc_setup

# tc_add_user_or_break    # in addition to standard setup, create a temporary user

cleanup
test01
# test02
test03
cleanup

# If you want a sequence of tests to each be dependent on the previous one
# having succeeded, chain them together with "&&" as follows ...
#
# test01 && \
# test02 && \
# test03 && \
# test04
