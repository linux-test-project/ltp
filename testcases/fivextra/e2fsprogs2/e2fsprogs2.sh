#!/bin/bash
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		                                      ##
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
# File :	e2fsprogs.sh
#
# Description:	Test e2fsprogs package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Mar 06 2003 - Created - RR
#		Mar 11 2003 - Converted to use tc_utils.source functions (rcp)
#		MAr 16 2003 - Now copies tests to $TCTMP before executing
#		Oct 13 2003 - fix a possible overflow of tst_resm on long
#				error messages. -RR
#               Oct 21 2003 - complete rewrite -RR
#		08 Jan 2004 - (RR) updated to tc_utils.source
#		26 Jan 2004 - (RR) remove f_swapfs test from runlist.
#			https://bugzilla.linux.ibm.com/show_bug.cgi?id=4752
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

# variables exported to interface with the various test scripts

export test_dir=""  # set by test_script()
export test_name="" # set by test_script()
export cmd_dir="."  # required by individual tests

# required executables:
REQUIRED="which dd cat chattr cmp head ls lsattr mke2fs rm sed"

# tests to execute
TESTS="d_loaddump \
	f_bad_local_jnl \
	f_baddir \
	f_baddotdir \
	f_badprimary \
	f_bitmaps \
	f_dirlink \
	f_end-bitmap \
	f_ext_journal \
	f_holedir \
	f_journal \
	f_okgroup \
	f_orphan \
	f_preen \
	f_reconnect \
	f_zero_group \
	f_zero_super"


################################################################################
# utility functions
################################################################################

#
# Setup specific to this testcase
#
function tc_local_setup()
{
	tc_exec_or_break $REQUIRED || return
}

################################################################################
# testcase functions
################################################################################

# Function:		test_script
#
# Description:		- exercise modified e2fsprogs "make check" tests
#
# Parameters:		
#
# Return		- zero on success
#			- return value from commands on failure
#
function test_script() {

	for test_dir in $TESTS ; do

		[ -d $test_dir ]
		tc_break_if_bad "$?" "The test directory $test_dir does not exist." || continue

		test_name=`echo $test_dir | sed -e 's;.*/;;'`
		tc_register "$test_name"

		if [ -f $test_dir/name ]; then
			test_description=`head $test_dir/name`
			tc_info "$test_description"
		fi
		if [ -f $test_dir/script ]; then
			source $test_dir/script 2>$stderr
			if [ -f $test_dir.failed ] ; then 
				cat $test_dir.failed >>$stderr
				[ -f $test_dir.out ] && cat $test_dir.out >$stdout
			fi
			[ -f $test_dir.ok ]
			tc_pass_or_fail "$?" "Unexpected results from $test_name."

		else
			test_base=`echo $test_name | sed -e 's/_.*//'`
			default_script=defaults/${test_base}_script
			[ -f $default_script ]
			tc_break_if_bad "$?" "Missing test script." || continue
			source $default_script 2>$stderr
                        if [ -f $test_dir.failed ] ; then
				cat $test_dir.failed >>$stderr
				[ -f $test_dir.out ] && cat $test_dir.out >$stdout
			fi
                        [ -f $test_dir.ok ]
                        tc_pass_or_fail "$?" "unexpected results from $description."
		fi
	done
}

################################################################################
# MAIN
################################################################################

# Function:	main
#
# Description:	- Execute all tests, report results
#
# Exit:		- zero on success
#		- non-zero on failure
#

set $TESTS; TST_TOTAL=$#
tc_setup
source test_config
test_script

