#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2001						      ##
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
# File :	diff.sh
#
# Description:	This testcase checks a collection of the following 
#		diff command options.
#
# Author:	Shoji Sugiyama (shoji@jp.ibm.com)
#
# History:	Mar 22 2003 - Initial version created. 
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
# test01	diff -b
#
function test01()
{
	#
	# Register test case
	#
	tc_register "\"diff -b\" check -b option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test files
	#
	echo "abcd" > $TCTMP/file1
	echo "abcd   " > $TCTMP/file2
	
	#
	# Execute and check result
	#
	diff -b $TCTMP/file1 $TCTMP/file2
	tc_pass_or_fail $? "[diff -b] failed with some white space at the end of line"
}

#
# test02	diff -c
#
function test02()
{
	#
	# Register test case
	#
	tc_register "\"diff -c\" check -c option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test files
	#
	echo -e "00\n11\n22\n33\n44\n55\n66\n77\n88\n99" > $TCTMP/file1
	echo -e "00\n11\n22\n33\n44\n55x\n66\n77\n88\n99" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	diff -c $TCTMP/file1 $TCTMP/file2 > $TCTMP/file3
	rc=$?
	[ $rc != 0 ]
	tc_fail_if_bad $? "[diff -c] #1 unexpected result rc=$rc" || return
	grep " 3,9 " $TCTMP/file3 > /dev/null
	tc_fail_if_bad $? "[diff -c] #2 unexpected result" || return
	grep "! 55" $TCTMP/file3 > /dev/null
	tc_pass_or_fail $? "[diff -c] #3 unexpected result"
}

#
# test03	diff -e
#
function test03()
{
	#
	# Register test case
	#
	tc_register "\"diff -e\" check -e option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo cat || return
	
	#
	# Prepare test files
	#
	echo -e "00\n11\n22\n33\n44\n55\n66\n77\n88\n99" > $TCTMP/file1
	echo -e "00\n11\n22x\n33\n44\n55\n66\n77\n88x\n99" > $TCTMP/file2
	echo -e "9c\n88x\n.\n3c\n22x\n." > $TCTMP/file3
	
	#
	# Execute and check result
	#
	diff -e $TCTMP/file1 $TCTMP/file2 > $TCTMP/file4
	rc=$?
	[ $rc != 0 ]
	tc_fail_if_bad $? "[diff -e] #1 unexpected result" || return
	diff $TCTMP/file3 $TCTMP/file4 > /dev/null
	tc_pass_or_fail $? "[diff -e] #2 unxpected result with differences in restored file by using ed"
}

#
# test04	diff -f
#
function test04()
{
	#
	# Register test case
	#
	tc_register "\"diff -f\" check -f option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo cat || return
	
	#
	# Prepare test and expected result files
	#
	echo -e "00\n11\n22\n33\n44\n55\n66\n77\n88\n99" > $TCTMP/file1
	echo -e "00\n11\n22x\n33\n44\n55\n66\n77\n88x\n99" > $TCTMP/file2
	echo -e "c3\n22x\n.\nc9\n88x\n." > $TCTMP/file3
	
	#
	# Execute and check result
	#
	diff -f $TCTMP/file1 $TCTMP/file2 > $TCTMP/file4
	rc=$?
	[ $rc != 0 ]
	tc_fail_if_bad $? "[diff -f] #1 unexpected result" || return
	diff $TCTMP/file3 $TCTMP/file4 > /dev/null
	tc_pass_or_fail $? "[diff -f] #2 unexpected result"
}

#
# test05	diff -r
#
function test05()
{
	#
	# Register test case
	#
	tc_register "\"diff -f\" check -f option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo cat mkdir || return
	
	#
	# Prepare test and expected result files
	#
	mkdir -p $TCTMP/dir1/dirA
	mkdir -p $TCTMP/dir2/dirA
	echo -e "00\n11\n22\n33\n44\n55\n66\n77\n88\n99" > $TCTMP/dir1/dirA/file1
	echo -e "00\n11\n22\n33\n44\n55\n66\n77\n88\n99" > $TCTMP/dir2/dirA/file1
	
	#
	# Execute and check result
	#
	diff -r $TCTMP/dir1 $TCTMP/dir2 > /dev/null
	tc_fail_if_bad $? "[diff -r] #1 : failed with unexpected result" || return

	# Check if subdir name is di
	mkdir -p $TCTMP/dir1/dirB
	diff -r $TCTMP/dir1 $TCTMP/dir2 > /dev/null
	rc=$?
	[ $rc != 0 ]
	tc_pass_or_fail $? "[diff -r] #2 : failed with unexpected result"
}

#
# test06	diff -C
#
function test06()
{
	#
	# Register test case
	#
	tc_register "\"diff -C\" check -C option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo cat mkdir || return
	
	#
	# Prepare test and expected result files
	#
	echo -e "00\n11\n22\n33\n44\n55\n66\n77\n88\n99" > $TCTMP/file1
	echo -e "00\n11\n22x\n33\n44\n55\n66x\n77\n88\n99" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	diff -C 1 $TCTMP/file1 $TCTMP/file2 > $TCTMP/file3
	num=`grep -- "---"$ $TCTMP/file3 | wc -l`
	[ $num = 2 ]
	tc_pass_or_fail $? "[diff -C] #1 : failed with unxpected result"
}

################################################################################
# main
################################################################################

TST_TOTAL=6

# standard setup
tc_setup

# tc_add_user_or_break    # in addition to standard setup, create a temporary user

test01
test02
test03
test04
test05
test06

# If you want a sequence of tests to each be dependent on the previous one
# having succeeded, chain them together with "&&" as follows ...
#
# test01 && \
# test02 && \
# test03 && \
# test04
