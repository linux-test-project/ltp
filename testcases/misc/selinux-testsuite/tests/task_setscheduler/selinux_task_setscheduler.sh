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
	LTPTMP="/tmp/selinux"
	export TCID="setup"
	export TST_COUNT=0

	# Start the process that will have its priority and scheduling changed.
	runcon -t test_setsched_target_t selinux_task_setscheduler_target &
	PID=$!
	sleep 1 # Give it a second to start
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_setsched_yes_t can change the scheduling.
	# SCHED_OTHER	0	priority must == 0
	# SCHED_FIFO	1	priority 1..99
	# SCHED_RR	2	priority 1..99
	runcon -t test_setsched_yes_t -- selinux_task_setscheduler_source $PID 2 1 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "Test #1: task_setscheduler passed."
	else
		echo "Test #1: task_setscheduler failed."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_setsched_no_t cannot change the scheduling.
	runcon -t test_setsched_no_t -- selinux_task_setscheduler_source $PID 2 1 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #2: task_setscheduler passed."
		return 0
	else
		echo "Test #2: task_setscheduler failed."
		return 1
	fi
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

setup  || exit $RC
test01 || exit $RC
test02 || exit $RC
cleanup
exit 0
