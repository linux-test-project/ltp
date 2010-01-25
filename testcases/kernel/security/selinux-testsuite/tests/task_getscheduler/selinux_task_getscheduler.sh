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
	export TST_TOTAL=2

	# Start the target process.
	runcon -t test_getsched_target_t selinux_task_getscheduler_target &
	PID=$!
	sleep 1 # Give it a second to start
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_getsched_yes_t can get the scheduling.
	# SCHED_OTHER	0	priority must == 0
	# SCHED_FIFO	1	priority 1..99
	# SCHED_RR	2	priority 1..99
	runcon -t test_getsched_yes_t -- selinux_task_getscheduler_source $PID 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "task_getscheduler passed."
	else
		tst_resm TFAIL "task_getscheduler failed."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_getsched_no_t cannot get the scheduling.
	runcon -t test_getsched_no_t -- selinux_task_getscheduler_source $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "task_getscheduler passed."
		RC=0
	else
		tst_resm TFAIL "task_getscheduler failed."
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
cleanup
exit $EXIT_VAL
