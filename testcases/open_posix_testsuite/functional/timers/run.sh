#!/bin/sh
# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.
#
# Run all the tests in the functional/timers area.

# Helper functions
RunTest()
{
	echo "TEST: " $1
	$1
	if [ $? == 0 ]; then
		PASS=$PASS+1
		echo -ne "\t\t\t***TEST PASSED***\n\n"
	else
		FAIL=$FAIL+1
		echo -ne "\t\t\t***TEST FAILED***\n\n"
	fi
}

# Main program

declare -i PASS=0
declare -i FAIL=0

# Add lists of tests to these variables for execution
CLOCKSTESTS="clocks/twopsetclock.test"
TIMERSTESTS="timers/twoevtimers.test timers/twoptimers.test"

echo "Run the clocks and timers functional tests"
echo "=========================================="

echo "Run clocks tests"
echo "================"

for test in $CLOCKSTESTS; do 
	RunTest $test
done

echo "Run timers tests"
echo "================"

for test in $TIMERSTESTS; do 
	RunTest $test
done

echo
echo -ne "\t\t\t****************\n"
echo -ne "\t\t\t* PASSED: " $PASS "\n"
echo -ne "\t\t\t* FAILED: " $FAIL "\n"
echo -ne "\t\t\t****************\n"

exit 0
