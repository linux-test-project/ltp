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
# File :	sdiff.sh
#
# Description:	This testcase checks a collection of the following 
#		options of 'sdiff' command.
#
#		-i
#		-W
#		-b
#		-B
#		-I
#		-a
#		-w
#		-l
#		-s
#		-t
#		-d
#		-H
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
# test01	sdiff -i
#
function test01()
{
	#
	# Register test case
	#
	tc_register "\"sdiff -i\" check -i option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo || return
	
	#
	# Prepare test files
	#
	echo "abcd" > $TCTMP/file1
	echo "aBcD" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	sdiff -i $TCTMP/file1 $TCTMP/file2 > /dev/null
	tc_pass_or_fail $? "[sdiff -i] failed with mixed case characters"
}

#
# test02	sdiff -W
#
function test02()
{
	#
	# Register test case
	#
	tc_register "\"sdiff -W\" check -W option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test files
	#
	echo -e "abcd" > $TCTMP/file1
	echo -e " a b c   d   " > $TCTMP/file2
	
	#
	# Execute and check result
	#
	sdiff -W $TCTMP/file1 $TCTMP/file2 > /dev/null
	tc_pass_or_fail $? "[sdiff -W] failed with white space characters"
}

#
# test03	sdiff -b
#
function test03()
{
	#
	# Register test case
	#
	tc_register "\"sdiff -b\" check -b option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo || return

	#
	# Prepare test files
	#
	echo -e "00\n11\n22\n33\n44\n55\n66\n77\n88\n99" > $TCTMP/file1
	echo -e "00\n11 \n22\n33  \n44\n55  \n66\n77\n88 \n99" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	sdiff -b $TCTMP/file1 $TCTMP/file2 > /dev/null
	tc_pass_or_fail $? "[sdiff -b] failed to ignore changes in the amount of white space"
}

#
# test04	sdiff -B
#
function test04()
{
	#
	# Register test case
	#
	tc_register "\"sdiff -B\" check -B option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo || return
	
	#
	# Prepare test and expected result files
	#
	echo -e "00\n11\n22\n33\n44\n55\n66\n77\n88\n99" > $TCTMP/file1
	echo -e "00\n11\n22\n\n33\n44\n\n55\n66\n77\n\n88\n99" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	sdiff -B $TCTMP/file1 $TCTMP/file2 > /dev/null
	tc_pass_or_fail $? "[sdiff -B] failed with blank lines"
}

#
# test05	diff -w
#
function test05()
{
	#
	# Register test case
	#
	tc_register "\"sdiff -w\" check -w option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo || return
	
	#
	# Prepare test and expected result files
	#
	echo -e "1234567890" > $TCTMP/file1
	echo -e "12345x67890y" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	sdiff -w 27 $TCTMP/file1 $TCTMP/file2 | grep -v y > /dev/null
	tc_fail_if_bad $? "[sdiff -w] #1 failed with specified num chars per line" || return
	
	sdiff -w 21 $TCTMP/file1 $TCTMP/file2 | grep -v x > /dev/null
	tc_pass_or_fail $? "[sdiff -w] #2 failed with specified num chars per line"
}

#
# test06	sdiff -l
#
function test06()
{
	#
	# Register test case
	#
	tc_register "\"sdiff -l\" check -l option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep wc || return
	
	#
	# Prepare test and expected result files
	#
	echo -e "00\n11\n22\n33\n44\n55\n66\n77\n88\n99" > $TCTMP/file1
	echo -e "00\n11\n22x\n33\n44\n55\n66x\n77\n88\n99" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	sdiff -l $TCTMP/file1 $TCTMP/file2 > $TCTMP/file3
	num=`grep "(" $TCTMP/file3 | wc -l`
	if [ $num -ne 8 ]; then
		# fail
		tc_pass_or_fail 1 "[sdiff -l] #1 : failed with unxpected result"
	else
		# success
		tc_pass_or_fail 0 "[sdiff -l] #1 : failed with unxpected result"
	fi	
}

#
# test07	sdiff -s
#
function test07()
{
	#
	# Register test case
	#
	tc_register "\"sdiff -s\" check -s option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test and expected result files
	#
	echo -e "00\n11\n22\n33\n44\n55\n66\n77\n88\n99" > $TCTMP/file1
	echo -e "00\n11\n22x\n33\n44\n55\n66x\n77\n88x\n99" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	sdiff -s $TCTMP/file1 $TCTMP/file2 > $TCTMP/file3
	num=`cat $TCTMP/file3 | wc -l`
	if [ $num -ne 3 ]; then
		# fail
		tc_pass_or_fail 1 "[sdiff -s] #1 : failed with suppress common line option"
	else
		# success
		tc_pass_or_fail 0 "[sdiff -s] #1 : failed with suppress common line option"
	fi	
}

#
# test07	sdiff -t
#
function test08()
{
	#
	# Register test case
	#
	tc_register "\"sdiff -t\" check -t option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test and expected result files
	#
	echo -e "00\n11\n22\n33\n44\n55\n66\n77\n88\n99" > $TCTMP/file1
	echo -e "00\n11\n22x\n33\n44\n55\n66x\n77\n88x\n99" > $TCTMP/file2
	
	#
	# Execute and check result
	#
	sdiff -t $TCTMP/file1 $TCTMP/file2 > $TCTMP/file3
	grep -v "	" $TCTMP/file3 > /dev/null
	tc_pass_or_fail $? "[sdiff -t] failed with expand tab option"
}

################################################################################
# main
################################################################################

TST_TOTAL=8

# standard setup
tc_setup

# tc_add_user_or_break    # in addition to standard setup, create a temporary user

test01
test02
test03
test04
test05
test06
test07
test08

# If you want a sequence of tests to each be dependent on the previous one
# having succeeded, chain them together with "&&" as follows ...
#
# test01 && \
# test02 && \
# test03 && \
# test04
