#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
##									      ##
## This program is free software;  you can redistribute it and/or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	rdate.sh
#
# Description:	Test rdate package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Jun 27 2003 - created - RR
#               Jul 14 2003 - modified per peer review -RR
#
#		07 Jan 2004 - (RR) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

SERVER="austime1.austin.ibm.com"

################################################################################
# testcase functions
################################################################################

function test01 {
	tc_register "rdate installed?"
	tc_executes rdate
	tc_pass_or_fail $? "rdate is not properly installed" || exit
}

function test02 {
	tc_register "rdate query"
	rdate -u $SERVER 1>$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from rdate command" || return
	[ -s $stdout ]
	tc_pass_or_fail $? "rdate did not write anything to stdout"
}

function test03 {
        tc_register "rdate set"
	rdate -s -u $SERVER 1>$stdout 2>$stderr
        tc_pass_or_fail $? "Unexpected response from rdate command"
}

####################################################################################
# MAIN
####################################################################################

# Function:	main
# - Execute test functions
#
# Exit:		- zero on success
#		- non-zero on failure
#
TST_TOTAL=3
tc_setup
tc_root_or_break || exit
test01 &&
test02 &&
test03
