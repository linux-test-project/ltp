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
# File :	cshost.sh
#
# Description:	This testcase checks 'cshost' command of canna.
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
	tc_register "\"cshost\" Display all host name connected to a canna server"

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo diff cshost || return
	
	#
	# Execute and check result
	#
	cshost > $TCTMP/file1
	tc_fail_if_bad $? "[cshost] Failed with unexpected return code."
	
	echo -e "Connected to unix\naccess control enabled\nHOST NAME:localhost\nALL USER\n\nHOST NAME:unix\nALL USER\n" > $TCTMP/file2
	diff -b $TCTMP/file1 $TCTMP/file2 > /dev/null 2>&1 
	tc_pass_or_fail $? "[cshost] Failed with invalid status."
}

################################################################################
# main
################################################################################

TST_TOTAL=1

# standard setup
tc_setup

# tc_add_user_or_break    # in addition to standard setup, create a temporary user

test01

# If you want a sequence of tests to each be dependent on the previous one
# having succeeded, chain them together with "&&" as follows ...
#
# test01 && \
# test02 && \
# test03 && \
# test04
