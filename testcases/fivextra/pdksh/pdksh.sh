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
# File :	pdksh.sh
#
# Description:	Test pdksh package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Jul 29 2003 - created - RR
#
#		07 Jan 2004 - (RR) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variables
REQUIRED="perl"
TESTS=""
TESTDIR="pdksh_tests"
KSH="/bin/ksh"
SCENARIO="pdksh,ksh,posix,posix-upu"

# Will be reset by pdksh_harness.pl
TST_TOTAL=1

################################################################################
# testcase functions
################################################################################

function test_01 {
    tc_register "Test that pdksh is installed"
    tc_executes "pdksh"
    tc_pass_or_fail $? "pdksh package is not properly installed" || exit
}


function runtest {

    ./$TESTDIR/pdksh_harness.pl -s ./$TESTDIR -p $KSH -C $SCENARIO -v

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
tc_setup
tc_exec_or_break $REQUIRED || exit 1
test_01 &&
runtest
