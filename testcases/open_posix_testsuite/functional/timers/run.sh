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
	: $(( TOTAL += 1 ))
	$1
	if [ $? -eq 0 ]; then
		: $(( PASS += 1 ))
		echo "			***TEST PASSED***"
		echo ""
	else
		: $(( FAIL += 1 ))
		echo "			***TEST FAILED***"
		echo ""
	fi
}

# Main program

TOTAL=0
PASS=0
FAIL=0

# Add lists of tests to these variables for execution
CLOCKSTESTS="clocks/twopsetclock.test"
TIMERSTESTS="timers/twoevtimers.test timers/twoptimers.test"

echo "Run the clocks and timers functional tests"
echo "====================="

echo "Run clocks tests"
echo "========"

for test in $CLOCKSTESTS; do 
	RunTest $test
done

echo "Run timers tests"
echo "========"

for test in $TIMERSTESTS; do 
	RunTest $test
done

cat <<EOF
		****************
		* TOTAL:  $TOTAL
		* PASSED: $PASS
		* FAILED: $FAIL
		****************
EOF

exit 0
