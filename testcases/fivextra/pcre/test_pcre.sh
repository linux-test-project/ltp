#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      ##
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
# File :	test_pcre.sh
#
# Description:	Test pcre/libpcre package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Jul 31 2003 - Created - Robb
#
#		07 Jan 2004 - (RR) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

REQUIRED="diff"
testdata=./pcre_testdata


################################################################################
# testcase functions
################################################################################

function test01 {
    tc_register "Is PCRE installed?"
    tc_executes pcre-config pcregrep pcretest
    tc_pass_or_fail $? "PCRE is not properly installed."
}

function test02 {
    tc_register "Testing main PCRE functionality (Perl compatible)."
    pcretest $testdata/testinput1 $TCTMP/testtry1 2>$stderr 1>$stdout
    tc_fail_if_bad $? "pcretest did not execute correctly." || return
    diff $TCTMP/testtry1 $testdata/testoutput1 2>$stderr 1>$stdout
    tc_pass_or_fail $? "Test data does not compare."
}


# PCRE tests that are not Perl-compatible - API & error tests, mostly
function test03 {
    tc_register "Testing PCRE API and error handling (not Perl compatible)."
    pcretest -i $testdata/testinput2 $TCTMP/testtry2 2>$stderr 1>$stdout
    tc_fail_if_bad $? "pcretest did not execute correctly." || return
    diff $TCTMP/testtry2 $testdata/testoutput2 2>$stderr 1>$stdout
    tc_pass_or_fail $? "Test data does not compare."
}


# Additional Perl-compatible tests for Perl 5.005's new features
function test04 {
    tc_register "Testing PCRE Perl 5.005 features (Perl 5.005 compatible)"
    pcretest $testdata/testinput3 $TCTMP/testtry3 2>$stderr 1>$stdout
    tc_fail_if_bad $? "pcretest did not execute correctly." || return
    diff $TCTMP/testtry3 $testdata/testoutput3 2>$stderr 1>$stdout
    tc_pass_or_fail $? "Test data does not compare."
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
TST_TOTAL=4
tc_setup
tc_exec_or_break $REQUIRED || exit
test01 &&
test02 &&
test03 &&
test04
