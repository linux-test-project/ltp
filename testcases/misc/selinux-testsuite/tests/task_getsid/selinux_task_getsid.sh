#!/bin/sh

setup()
{
	LTPTMP="/tmp/selinux"
	export TCID="setup"
	export TST_COUNT=0

	# Start the target process.
	runcon -t test_getsid_target_t selinux_task_getsid_target &
	
	PID=$!
	sleep 1; # Give it a second to start
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_getsid_yes_t can get the session ID.
	runcon -t test_getsid_yes_t -- selinux_task_getsid_source $PID 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "Test #1: task_getsid passed."
	else
		echo "Test #1: task_getsid failed."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_getsid_no_t cannot get the session ID.
	runcon -t test_getsid_no_t -- selinux_task_getsid_source $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #2: task_getsid passed."
		return 0
	else
		echo "Test #2: task_getsid failed."
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
