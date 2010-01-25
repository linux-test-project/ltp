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

setup()
{
	export TCID="setup"
	export TST_COUNT=0
	export TST_TOTAL=4

	# Start the process that will have its priority changed.
	runcon -t test_setsched_target_t selinux_task_setnice_target &
	PID=$!
	sleep 1 # Give it a second to start
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_setsched_yes_t can change the priority up and down.
	runcon -t test_setsched_yes_t -- renice +10 -p $PID 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "task_setnice passed."
        else
		tst_resm TFAIL "task_setnice failed."
        fi
        return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Giving the process a higher priority (lower nice number) 
	# requires the sys_nice capability
	runcon -t test_setsched_yes_t -- renice -20 -p $PID 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "task_setnice passed."
        else
		tst_resm TFAIL "task_setnice failed."
        fi
        return $RC
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	# Verify that test_setsched_no_t cannot change
	# the priority up or down.
	runcon -t test_setsched_no_t -- renice +10 -p $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "task_setnice passed."
		RC=0
        else
		tst_resm TFAIL "task_setnice failed."
		RC=1
        fi
	return $RC
}

test04()
{
	TCID="test04"
	TST_COUNT=4
	RC=0

	runcon -t test_setsched_no_t -- renice -20 -p $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "task_setnice passed."
		RC=0
        else
		tst_resm TFAIL "task_setnice failed."
		RC=1
        fi
	return $RC
}

cleanup()
{
	# Kill the target
	kill -s KILL $PID 
}

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, and test functions.
EXIT_VAL=0

setup 
test01 || EXIT_VAL=$RC
test02 || EXIT_VAL=$RC
test03 || EXIT_VAL=$RC
test04 || EXIT_VAL=$RC
cleanup
exit $EXIT_VAL
