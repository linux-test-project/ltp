#!/bin/sh
#
# Copyright (c) 2002 Network Associates Technology, Inc.
# Copyright (c) International Business Machines  Corp., 2005
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_create_yes_t can fork.
	runcon -t test_create_yes_t -- selinux_task_create_parent 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "Test #1: task_create passed."
	else
		echo "Test #1: task_create failed."
	fi
	return $RC
}

test02()
{

	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_create_no_t cannot fork.
	runcon -t test_create_no_t -- selinux_task_create_parent 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #2: task_create passed."
		return 0
	else
		echo "Test #2: task_create failed."
		return 1
	fi
}

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, and test functions.

test01 || exit $RC
test02 || exit $RC
exit 0
