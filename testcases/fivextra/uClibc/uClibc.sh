#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        uClibc.sh
#
# Description:  Test uClibc package
#
# Author:       
#
# History:      
#	
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# testcase functions
################################################################################

# Function:             runtests
#
# Description:          - test uClibc
#
# Parameters:           - none
#
# Return                - zero on success
#                       - return value from commands on failure
#

TESTDIR="uClibctest" # testdir we store individual test files in


function runtests() {

# tests is the list of our test suites
local tests=" md5c-test ctype sem mmap setjmp_test sigchld signal hello tiny ptytest"

set $tests
TST_TOTAL=$#

# run each testcase
for tst in $tests ; do

	# housekeeping
	tc_register "$tst"

        # make sure test file exists
	[ -f $TESTDIR/$tst ]
	tc_break_if_bad "$?" "The test '$TESTDIR/$tst' does not exist." || continue

	# runit
	$TESTDIR/$tst 2>$stderr >$stdout
	tc_pass_or_fail "$?" "$tst output unexpected."

done
}

function rundifftests() {

# tests is the list of our test suites
local tests=" getgroups grcat pwcat test_grp test_pwd teststrtol qsort teston_exit testatexit "

set $tests
TST_TOTAL=$#

# run each testcase
for tst in $tests ; do

	# housekeeping
	tc_register "$tst"

	# make sure test file exists
	[ -f $TESTDIR/$tst ]
	tc_break_if_bad "$?" "The test $TESTDIR/$tst does not exist." || continue

	[ -f $TESTDIR/${tst}_glibc ]
	tc_break_if_bad "$?" "The test $TESTDIR/$tst_glibc does not exist." || continue

        # run it
        $TESTDIR/$tst > $TCTMP/${tst}.out 2>&1
        $TESTDIR/${tst}_glibc > $TCTMP/${tst}_glibc.out 2>&1

        diff -u $TCTMP/${tst}_glibc.out $TCTMP/${tst}.out > /dev/null 2>&1
        tc_pass_or_fail "$?" "$tst's output unexpected."
				


done
}

function crypt_test() {

	tc_register "crypt"

        # make sure test file exists
        [ -f $TESTDIR/crypt ]
        tc_break_if_bad "$?" "The test $TESTDIR/crypt does not exist." || return 1

        [ -f $TESTDIR/crypt_glibc ]
        tc_break_if_bad "$?" "The test $TESTDIR/crypt_glibc does not exist." || return 1

        
        $TESTDIR/crypt < $TESTDIR/crypt.input > $TCTMP/crypt.out 2>&1 
        $TESTDIR/crypt_glibc < $TESTDIR/crypt.input > $TCTMP/crypt_glibc.out 2>&1 

        diff -u $TCTMP/crypt_glibc.out $TCTMP/crypt.out > /dev/null 2>&1
        tc_pass_or_fail "$?" "crypt's output unexpected."


}
										

function dirent_test() {

        tc_register "dirent"

        # make sure test file exists
        [ -f $TESTDIR/dirent ]
        tc_break_if_bad "$?" "The test $TESTDIR/dirent does not exist." || return 1

        [ -f $TESTDIR/dirent_glibc ]
        tc_break_if_bad "$?" "The test $TESTDIR/dirent_glibc does not exist." || return 1


        $TESTDIR/dirent < $TESTDIR/dirent.c > $TCTMP/dirent.out 2>&1
        $TESTDIR/dirent_glibc < $TESTDIR/dirent.c > $TCTMP/dirent_glibc.out 2>&1

        diff -u $TCTMP/dirent_glibc.out $TCTMP/dirent.out > /dev/null 2>&1
        tc_pass_or_fail "$?" "dirent's o's utput unexpected."


}
										
										




function stat_test() {

        tc_register "stat"

        # make sure test file exists
        [ -f $TESTDIR/stat ]
        tc_break_if_bad "$?" "The test $TESTDIR/stat does not exist." || return 1

        [ -f $TESTDIR/stat_glibc ]
        tc_break_if_bad "$?" "The test $TESTDIR/stat_glibc does not exist." || return 1

						        
        $TESTDIR/stat < $TESTDIR/stat.c > $TCTMP/stat.out 2>&1
        $TESTDIR/stat_glibc < $TESTDIR/stat.c > $TCTMP/stat_glibc.out 2>&1
							        
        diff -u $TCTMP/stat_glibc.out $TCTMP/stat.out > /dev/null 2>&1
        tc_pass_or_fail "$?" "stat's o's utput unexpected."


}
												

####################################################################################
# MAIN
####################################################################################

# Function:     main
#
# Description:  - Execute all tests, report results
#
# Exit:         - zero on success
#               - non-zero on failure
#
tc_setup
runtests
rundifftests
crypt_test
dirent_test
stat_test


