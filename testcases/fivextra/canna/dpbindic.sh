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
# File :	mkbindic.sh
#
# Description:	This testcase checks 'dpbindic' command of canna.
#
# Author:	Shoji Sugiyama (shoji@jp.ibm.com)
#
# History:	Jun 13 2003 - Initial version created. 
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

WORD1="\xa4\xa6\xa4\xef\xa4\xb5\xa4\xce\x20\x23\x54\x33\x35\x20\xb1\xbd\xa4\xce\xa3\xc3\xb8\xc0\xb8\xec"
WORD2="\xa4\xb9\xa4\xae\x20\x23\x4b\x4b\x20\xbf\xf9\xbb\xb3\xbe\xbb\xbc\xa3"
WORD3="\xa4\xb9\xa4\xae\x20\xbf\xf9\xbb\xb3\xbe\xbb\xbc\xa3"

#
# cleanup
#
function cleanup()
{
	#
	# Clear all test dict
	#
	for dic in `lsdic | fgrep _testdic`
	do
		rmdic $dic > /dev/null 2>&1
	done

	for dic in `lsdic -G | fgrep _testgdic`
	do
		rmdic -G $dic > /dev/null 2>&1
	done

	rm -rf $TCTMP/*
}

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
	tc_register "\"dpbindic\" Display binary dictionary information"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo mkbindic || return
	
	#
	# Execute and check result
	#
	# dpbindic assumes working directory should be the current directory
	# So we need to change directory to a temporary directory.

	cd $TCTMP
	echo -e $WORD1 > file1.ctd
	echo -e $WORD2 >> file1.ctd
	mkbindic  file1.ctd > /dev/null 2>&1
	tc_fail_if_bad $? "[dpbindic] Could not create a new binary dictionary"  || return

	dpbindic file1.cbd 2> file2
	# dpbindic does not return 0 even if it succeeded.
	# tc_fail_if_bad $? "[dpbindic] Failed with unexpected return code"  || return
	grep "2 + 2" file2 > /dev/null 2>&1
	tc_pass_or_fail $? "[dpbindic] Failed with unexpected dic info"  || return
}

#
# test02	
#
function test02()
{
	#
	# Register test case
	#
	tc_register "\"dpbindic\" Display binary dictionary info in detail"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo dpbindic || return
	
	#
	# Execute and check result
	#
	dpbindic file1.cbd file1.mwd > file2 2> /dev/null
	tc_fail_if_bad $? "[dpbindic] Failed with unexpected return code"  || return
 
	sdiff file2 file1.ctd > /dev/null 2>&1
	tc_pass_or_fail $? "[dpbindic] Failed with unexpected dic info"  || return
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
