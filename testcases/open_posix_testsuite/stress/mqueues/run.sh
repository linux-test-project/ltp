#!/bin/sh
# Copyright (c) 2002, Intel Corporation. All rights reserved.
# Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
# This file is licensed under the GPL license.  For the full content
# of this license, see the COPYING file at the top level of this
# source tree.
#
# Run all the tests in the message queues stress area.

# Helper functions
RunTest()
{
	echo "TEST: " $1 $2
	TOTAL=$TOTAL+1
	./$1 $2
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
TESTS="multi_send_rev_1.test multi_send_rev_2.test"

echo "Run the message queue stress tests"
echo "=========================================="

RunTest multi_send_rev_1.test 10
RunTest multi_send_rev_2.test 100
 

echo
echo -ne "\t\t****************\n"
echo -ne "\t\t* TOTAL:  " $TOTAL "\n"
echo -ne "\t\t* PASSED: " $PASS "\n"
echo -ne "\t\t* FAILED: " $FAIL "\n"
echo -ne "\t\t****************\n"

exit 0


