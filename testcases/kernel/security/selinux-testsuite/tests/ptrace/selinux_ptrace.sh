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

	# run tests in $LTPROOT/testcases/bin directory
	SAVEPWD=${PWD}
	cd ${LTPBIN}
	CURRENTDIR=.

	# Start the process to be traced.
	runcon -t test_ptrace_traced_t $CURRENTDIR/selinux_ptrace_wait.sh &
	PID=$!

	# Give the process a moment to initialize.
	sleep 1 
}

test01()
{
	TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that the nottracer domain cannot attach to the process.
	# Should fail on the ptrace permission check.

	runcon -t test_ptrace_nottracer_t $CURRENTDIR/selinux_ptrace $PID
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "ptrace passed."
		RC=0
        else
                tst_resm TFAIL "ptrace failed."
		RC=1
        fi
	return $RC
}

test02()
{
	TCID="test02"
        TST_COUNT=2
        RC=0

	# Verify that the tracer domain can trace to the process.
	runcon -t test_ptrace_tracer_t $CURRENTDIR/selinux_ptrace $PID
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "ptrace passed."
        else
                tst_resm TFAIL "ptrace failed."
        fi
	return $RC
}

cleanup()
{
	# Kill the process.
	kill -s KILL $PID

	# return to $LTPROOT directory
	cd $SAVEPWD
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
