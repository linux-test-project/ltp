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
# File :	diff3.sh
#
# Description:	This testcase checks a collection of the following 
#		options of 'diff3' command.
#
#		-e
#		-i
#		-m
#		-x
#		-A
#		-E
#		-L
#		-T
#		-X
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
# test01	diff3 -e
#
function test01()
{
	#
	# Register test case
	#
	tc_register "\"diff3 -e\" check -e option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo || return
	
	#
	# Prepare test files
	#
	echo -e "Axx\nxbx\nxxc\nABx\nAxC\nxbc\nABC" > $TCTMP/file1
	echo -e "axx\nxBx\nxxc\nABx\naxc\nxBC\nABC" > $TCTMP/file2
	echo -e "axx\nxbx\nxxC\nabx\nAxC\nxBC\nABC" > $TCTMP/file3
	echo -e "1,6c\naxx\nxbx\nxxC\nabx\nAxC\nxBC\n." > $TCTMP/file4

	
	#
	# Execute and check result
	#
	diff3 -e $TCTMP/file1 $TCTMP/file2 $TCTMP/file3 > $TCTMP/file5
	diff $TCTMP/file4 $TCTMP/file5 > /dev/null 2>&1
	tc_pass_or_fail $? "[diff3 -e] failed to generate correct ed command sequence"
}

#
# test02	diff3 -i
#
function test02()
{
	#
	# Register test case
	#
	tc_register "\"diff3 -i\" check -i option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo || return
	
	#
	# Prepare test files
	#
	echo -e "Axx\nxbx\nxxc\nABx\nAxC\nxbc\nABC" > $TCTMP/file1
	echo -e "axx\nxBx\nxxc\nABx\naxc\nxBC\nABC" > $TCTMP/file2
	echo -e "axx\nxbx\nxxC\nabx\nAxC\nxBC\nABC" > $TCTMP/file3
	echo -e "====\n1:1,6c\n  Axx\n  xbx\n  xxc\n  ABx\n  AxC\n  xbc\n2:1,6c\n  axx\n  xBx\n  xxc\n  ABx\n  axc\n  xBC\n3:1,6c\n  axx\n  xbx\n  xxC\n  abx\n  AxC\n  xBC" > $TCTMP/file4
	#
	# Execute and check result
	#
	diff3 -i $TCTMP/file1 $TCTMP/file2 $TCTMP/file3 > $TCTMP/file5
	diff $TCTMP/file4 $TCTMP/file5 > /dev/null 2>&1
	tc_pass_or_fail $? "[diff3 -i] failed to generate correct ed command sequence"
}

#
# test03	diff3 -m
#
function test03()
{
	#
	# Register test case
	#
	tc_register "\"diff3 -m\" check -m option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo || return
	
	#
	# Prepare test files
	#
	echo -e "line1\nline2\nline3\nline4\nline5" > $TCTMP/file1
	echo -e "line1\nline2 updated by file2\nline3\nline4 updated by file2\nline5" > $TCTMP/file2
	echo -e "line1\nline2\nline3 updated by file3\nline4 updated by file3\nline5" > $TCTMP/file3
	echo -e "line1\n<<<<<<< $TCTMP/file1\nline2\nline3\nline4\n||||||| $TCTMP/file2\nline2 updated by file2\nline3\nline4 updated by file2\n=======\nline2\nline3 updated by file3\nline4 updated by file3\n>>>>>>> $TCTMP/file3\nline5" > $TCTMP/file4
	
	#
	# Execute and check result
	#
	diff3 -m $TCTMP/file1 $TCTMP/file2 $TCTMP/file3 > $TCTMP/file5
	diff $TCTMP/file4 $TCTMP/file5 > /dev/null 2>&1
	tc_pass_or_fail $? "[diff3 -m] failed to generate correct ed command sequence"
}

#
# test04	diff3 -x
#
function test04()
{
	#
	# Register test case
	#
	tc_register "\"diff3 -x\" check -x option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test files
	#
	echo -e "line1\nline2\nline3\nline4\nline5" > $TCTMP/file1
	echo -e "line1\nline2 updated by file2\nline3 updated by file2\nline4\nline5" > $TCTMP/file2
	echo -e "line1 updated by file3\nline2\nline3\nline4\nline5 updated by file3" > $TCTMP/file3
	
	#
	# Execute and check result
	#
	diff3 -x $TCTMP/file1 $TCTMP/file2 $TCTMP/file3 > $TCTMP/file5
	grep "line5" $TCTMP/file5 > /dev/null
	[ $? = 1 ]
	tc_pass_or_fail $? "[diff3 -x] unexpected error"
}

#
# test05	diff3 -A
#
function test05()
{
	#
	# Register test case
	#
	tc_register "\"diff3 -A\" check -A option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test files
	#
	echo -e "line1\nline2\nline3\nline4" > $TCTMP/file1
	echo -e "line1\nline2 updated by file2\nline3\nline4 updated by file2" > $TCTMP/file2
	echo -e "line1\nline2\nline3 updated by file3\nline4 updated by file3" > $TCTMP/file3
	echo -e "4a\n||||||| $TCTMP/file2\nline2 updated by file2\nline3\nline4 updated by file2\n=======\nline2\nline3 updated by file3\nline4 updated by file3\n>>>>>>> $TCTMP/file3\n.\n1a\n<<<<<<< $TCTMP/file1\n." > $TCTMP/file4
	
	#
	# Execute and check result
	#
	diff3 -A $TCTMP/file1 $TCTMP/file2 $TCTMP/file3 > $TCTMP/file5
	diff $TCTMP/file4 $TCTMP/file5
	tc_pass_or_fail $? "[diff3 -A] failed with unexpected result"
}

#
# test06	diff3 -E
#
function test06()
{
	#
	# Register test case
	#
	tc_register "\"diff3 -E\" check -E option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo grep || return
	
	#
	# Prepare test files
	#
	echo -e "line1\nline2\nline3\nline4" > $TCTMP/file1
	echo -e "line1\nline2 updated by file2\nline3\nline4 updated by file2" > $TCTMP/file2
	echo -e "line1\nline2\nline3 updated by file3\nline4 updated by file3" > $TCTMP/file3
	echo -e "4a\n=======\nline2\nline3 updated by file3\nline4 updated by file3\n>>>>>>> $TCTMP/file3\n.\n1a\n<<<<<<< $TCTMP/file1\n." > $TCTMP/file4

	#
	# Execute and check result
	#
	diff3 -E $TCTMP/file1 $TCTMP/file2 $TCTMP/file3 > $TCTMP/file5  
	diff $TCTMP/file4 $TCTMP/file5
	tc_pass_or_fail $? "[diff3 -A] "
}

#
# test07	diff3 -L
#
function test07()
{
	#
	# Register test case
	#
	tc_register "\"diff3 -L\" check -L option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo sed || return
	
	#
	# Prepare test files
	#
	echo -e "Axx\nxbx\nxxc\nABx\nAxC\nxbc\nABC" > $TCTMP/file1
	echo -e "axx\nxBx\nxxc\nABx\naxc\nxBC\nABC" > $TCTMP/file2
	echo -e "axx\nxbx\nxxC\nabx\nAxC\nxBC\nABC" > $TCTMP/file3
	
	#
	# Execute and check result
	#
	diff3 -A $TCTMP/file1 $TCTMP/file2 $TCTMP/file3 > $TCTMP/file4
	diff3 -A -L $TCTMP/XXXXX -L $TCTMP/YYYYY -L $TCTMP/ZZZZZ $TCTMP/file1 $TCTMP/file2 $TCTMP/file3 > $TCTMP/file5
	cat $TCTMP/file5 | sed "s/XXXXX/file1/" | sed "s/YYYYY/file2/" | sed "s/ZZZZZ/file3/" > $TCTMP/file6
	diff $TCTMP/file4 $TCTMP/file6 > /dev/null 2>&1
	tc_pass_or_fail $? "[diff3 -L] failed to generate correct label"
}

#
# test08	diff3 -T
#
function test08()
{
	#
	# Register test case
	#
	tc_register "\"diff3 -T\" check -T option"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo || return
	
	#
	# Prepare test files
	#
	echo -e "line1\nline2\nline3\nline4\nline5" > $TCTMP/file1
	echo -e "line1\nline2 updated by file2\nline3\nline4 updated by file2\nline5" > $TCTMP/file2
	echo -e "line1\nline2\nline3 updated by file3\nline4 updated by file3\nline5" > $TCTMP/file3
	
	#
	# Execute and check result
	#
	diff3 $TCTMP/file1 $TCTMP/file2 $TCTMP/file3 | sed 's/^  /	/' > $TCTMP/file4
	diff3 -T $TCTMP/file1 $TCTMP/file2 $TCTMP/file3 > $TCTMP/file5
	diff $TCTMP/file4 $TCTMP/file5 > /dev/null 2>&1
	tc_pass_or_fail $? "[diff3 -T] unexpected error with initial tab"
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
