#! /bin/bash
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003						      ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        pythontest.sh
#
# Description:  Test suite to exhaustively test python
#		Exercises testcases shipped with python
#
# Author:       Robb Romans <robb@austin.ibm.com>
#
# History:      Feb 11 2003 - 1st version
#		Feb 27 2003 - 1st revision
#		Feb 04 2004 (rcp) Converted to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################
# required executables
REQUIRED="which cat grep ls python rm sed"

TFAILCOUNT=0				# test failures
TSKIPCOUNT=0				# tests skipped by $TEST_CMD harness

TEST_CMD="testcases/regrtest.py"	# name of test engine

RUN_STD_TESTS="yes"		# run the standard python tests
RUN_NET_TESTS=""		# run the tests that depend on networking ""=no
RUN_STRESS_TESTS=""		# run the tests that create large files ""=no

STD_TESTS="stdtests.txt"	# list of standard testcases, one per line
NET_TESTS="nettests.txt"	# list of network testcases, one per line
STRESS_TESTS="stresstests.txt"	# list of stress tescases, one per line
#TESTDIR="testcases"		# location of individual testcases

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_exec_or_break $REQUIRED || exit

	# Find total number of testcases
	TST_TOTAL=0
	[ "$RUN_STD_TESTS" == "yes"    ] && TST_TOTAL=$TST_TOTAL+$(cat $STD_TESTS   | wc -l)
	[ "$RUN_NET_TESTS" == "yes"    ] && TST_TOTAL=$TST_TOTAL$(cat $NET_TESTS    | wc -l)
	[ "$RUN_STRESS_TESTS" == "yes" ] && TST_TOTAL=$TST_TOTAL$(cat $STRESS_TESTS | wc -l)

	return 0
}

################################################################################
# utility functions
################################################################################

# Function:	passfail
#
# Description:	- pass or fail based on true/false input
#
# Parameters:	$1      tcname
#		$2	output file to check
#
function passfail()
{
        if [[ "$(grep OK $2)" ]] ; then
		tc_pass_or_fail 0 "$1"	# always passes
        elif [[ "$(grep skipped $2)" ]] ; then
		if [[ "$(grep expected $2)" ]] ; then
			(( TSKIPPEDCOUNT+=1 ))
                	tc_info "$1 has been skipped on this platform (expected)."
		else
			(( TFAILCOUNT+=1 ))
			tc_pass_or_fail 1 "Failed"$'\n'"$(cat $2)"
		fi
        else
                (( TFAILCOUNT+=1 ))
		tc_pass_or_fail 1 "Failed"$'\n'"$(cat $2)"
        fi
}

################################################################################
# testcase functions
################################################################################

# Function run_test
#
# Description	- run all python standard testcases
#
# Parameters:	- $1 testcase command
#		- $2 testcase to run
#
# Return	- zero on success
#		- return value from testcase on failure ($RC)

function run_test {
	rm -f $TCTMP/$TCNAME.out
	(( TST_COUNT+=1 ))
	$1 $2 >& $TCTMP/$TCNAME.out
	passfail $2 $TCTMP/$TCNAME.out
}

################################################################################
# main
################################################################################
tc_setup

if [ "$RUN_STD_TESTS" == "yes" ] ; then
	WHICH_CMD=$TEST_CMD
	WHICH_TESTS=$STD_TESTS
	tc_info "$TCID: Starting standard tests."
	for line in `cat "$WHICH_TESTS"` ; do
		TCNAME=$line
		run_test $WHICH_CMD $line
	done
else
        tc_info "$TCID: Standard tests skipped."
fi

if [ "$RUN_NET_TESTS" == "yes" ] ; then
	WHICH_CMD="$TEST_CMD -u network"
	WHICH_TESTS=$NET_TESTS
	tc_info "$TCID: Starting network tests."
	for line in `cat "$WHICH_TESTS"` ; do
        	TCNAME=$line
		run_test $WHICH_CMD $line
	done
else
	tc_info "$TCID: Network tests skipped."
fi

if [ "$RUN_STRESS_TESTS" == "yes" ] ;  then
	WHICH_TESTS=$STRESS_TESTS
	WHICH_CMD="$TEST_CMD -u stress"
	tc_info "$TCID: Starting network tests."
        for line in `cat "$WHICH_TESTS"` ; do
                TCNAME=$line
                run_test $WHICH_CMD $line
	done
else
	tc_info "$TCID: Stress tests skipped."
fi

tc_info "$TFAILCOUNT tests failed"
tc_info "$TSKIPPEDCOUNT tests skipped (expected)."
