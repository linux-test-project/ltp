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
# File :	cmp.sh
#
# Description:	This testcase checks a collection of the following 
#		options of 'cmp' command.
#
#		-c
#		-i
#		-l
#		-s
#
# Author:	Shoji Sugiyama (shoji@jp.ibm.com)
#
# History:	Mar 28 2003 - Initial version created. 
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
# test01	cmp -c
#
function test01()
{
	#
	# Register test case
	#
	tc_register "\"cmp -c\" check -c option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test files
	#
	echo -e "\x01\x02\x03\x04\x05\x06\x07\x08\x09" > $TCTMP/file1
	echo -e "\x01\x02\x03\x14\x05\x06\x17\x08\x09" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	cmp -c $TCTMP/file1 $TCTMP/file2 > $TCTMP/file3
	grep "differ:" $TCTMP/file3 | grep "4, line 1 is   4 \^D  24 \^T" $TCTMP/file3 > /dev/null
	tc_pass_or_fail $? "[cmp -c] failed with mixed case characters"
}

#
# test02	cmp -i
#
function test02()
{
	#
	# Register test case
	#
	tc_register "\"cmp -i\" check -i option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test files
	#
	echo -e "\x01\x02\x03\x04\x05\x06\x07\x08\x09" > $TCTMP/file1
	echo -e "\x01\x02\x03\x14\x05\x06\x17\x08\x09" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	cmp -i 7 $TCTMP/file1 $TCTMP/file2 > /dev/null
	tc_pass_or_fail $? "[cmp -i] unexpected result"
}

#
# test03	cmp -l
#
function test03()
{
	#
	# Register test case
	#
	tc_register "\"cmp -l\" check -l option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test files
	#
	echo -e "\x01\x02\x03\x04\x05\x06\x07\x08\x09" > $TCTMP/file1
	echo -e "\x01\x02\x03\x14\x05\x06\x17\x08\x09" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	cmp -l $TCTMP/file1 $TCTMP/file2 > $TCTMP/file3
	num=`cat $TCTMP/file3 | wc -l`
	[ $num = 2 ]
	tc_pass_or_fail $? "[cmp -l] unexpected result with muliple differing bytes"
}


#
# test04	cmp -s
#
function test04()
{
	#
	# Register test case
	#
	tc_register "\"cmp -l\" check -l option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test files
	#
	echo -e "\x01\x02\x03\x04\x05\x06\x07\x08\x09" > $TCTMP/file1
	echo -e "\x01\x02\x03\x14\x05\x06\x17\x08\x09" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	cmp -s $TCTMP/file1 $TCTMP/file2 > $TCTMP/file3
	[ $? != 0 ]
	tc_fail_if_bad $? "[cmp -s] unexpected result with no differing bytes" || return
	num=`cat $TCTMP/file3 | wc -c`
	[ $num = 0 ]
	tc_pass_or_fail $? "[cmp -s] failed with unexpected output"
}

################################################################################
# main
################################################################################

TST_TOTAL=4

# standard setup
tc_setup

# tc_add_user_or_break    # in addition to standard setup, create a temporary user

test01
test02
test03
test04

# If you want a sequence of tests to each be dependent on the previous one
# having succeeded, chain them together with "&&" as follows ...
#
# test01 && \
# test02 && \
# test03 && \
# test04
