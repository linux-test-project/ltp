#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003							##
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
# File :	sharutils.sh
#
# Description:	Test sharutils package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Aug 4 2003 - created - RR
#		08 Jan 2004 - (RR) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

REQUIRED="cmp"

################################################################################
# testcase functions
################################################################################

function test01 {
	tc_register "Is sharutils installed?"
	tc_executes uudecode uuencode
	tc_pass_or_fail $? "Sharutils is not properly installed"
}


function test02 {
	tc_register "Test decode and encode functions"
        rm -f test.bin test.tmp
        uudecode sharutils.data
	tc_fail_if_bad $? "Unexpected response from uudecode command" || return
        uuencode test.bin test.bin > test.tmp
	tc_fail_if_bad $? "Unexpected response from uuencode command" || return
        cmp sharutils.data test.tmp 1>$stdout 2>$stderr
	tc_pass_or_fail $? "Unexpected output from compare"
        rm -f test.bin test.tmp
}

####################################################################################
# MAIN
####################################################################################

# Function:	main
#

#
# Exit:		- zero on success
#		- non-zero on failure
#
TST_TOTAL=2
tc_setup
tc_exec_or_break $REQUIRED || exit
test01 && \
test02
