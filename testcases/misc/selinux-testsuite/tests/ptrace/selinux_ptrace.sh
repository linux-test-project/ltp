#!/bin/sh

setup()
{
	LTPTMP="/tmp/selinux"
	export TCID="setup"
	export TST_COUNT=0

	SAVEPWD=${PWD}
	cd $LTPROOT/testcases/bin
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
                echo "Test #1: ptrace passed."
		return 0
        else
                echo "Test #1: ptrace failed."
		return 1
        fi
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
                echo "Test #2: ptrace passed."
        else
                echo "Test #2: ptrace failed."
        fi
	return $RC
}

cleanup()
{
	# Kill the process.
	kill -s KILL $PID
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

setup  || exit $RC
test01 || exit $RC
test02 || exit $RC
cleanup
exit 0
