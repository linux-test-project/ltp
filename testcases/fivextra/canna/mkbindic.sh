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
# Description:	This testcase checks 'addwords' command of canna.
#
# Author:	Shoji Sugiyama (shoji@jp.ibm.com)
#
# History:	Jun 12 2003 - Initial version created. 
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
	tc_register "\"mkbindic\" Make a new binary dictionary"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo mkbindic || return
	
	#
	# Execute and check result
	#
	echo -e $WORD1 > $TCTMP/file1.ctd
	echo -e $WORD2 >> $TCTMP/file1.ctd
	mkbindic  $TCTMP/file1.ctd > /dev/null 2>&1
	tc_pass_or_fail $? "[mkbindic] Could not create a new binary dictionary" 
}

################################################################################
# main
################################################################################

TST_TOTAL=1

# standard setup
tc_setup

# tc_add_user_or_break    # in addition to standard setup, create a temporary user

cleanup
test01
cleanup

# If you want a sequence of tests to each be dependent on the previous one
# having succeeded, chain them together with "&&" as follows ...
#
# test01 && \
# test02 && \
# test03 && \
# test04
