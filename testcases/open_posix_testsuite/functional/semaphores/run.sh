#!/bin/sh
# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.
#
# Run all the tests in the semaphore functional area.

# Helper functions
RunTest()
{
	echo "TEST: " $1
	TOTAL=$TOTAL+1
	./$1
	if [ $? == 0 ]; then
		PASS=$PASS+1
		echo -ne "\t\t\t***TEST PASSED***\n\n"
	else
		FAIL=$FAIL+1
		echo -ne "\t\t\t***TEST FAILED***\n\n"
	fi
}

# Main program

declare -i TOTAL=0
declare -i PASS=0
declare -i FAIL=0

# Add lists of tests to these variables for execution
TESTS="sem_conpro.test sem_lock.test sem_philosopher.test sem_readerwriter.test sem_sleepingbarber.test"

echo "Run the semaphore functional tests"
echo "=========================================="

for test in $TESTS; do 
	RunTest $test
done

echo
echo -ne "\t\t****************\n"
echo -ne "\t\t* TOTAL:  " $TOTAL "\n"
echo -ne "\t\t* PASSED: " $PASS "\n"
echo -ne "\t\t* FAILED: " $FAIL "\n"
echo -ne "\t\t****************\n"

exit 0


