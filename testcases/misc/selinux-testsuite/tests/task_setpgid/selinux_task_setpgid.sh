#!/bin/sh

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_setpgid_yes_t can setpgid.
	runcon -t test_setpgid_yes_t -- selinux_task_setpgid_source 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "Test #1: task_setpgid passed."
	else
		echo "Test #1: task_setpgid failed."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_setpgid_no_t cannot setpgid.
	runcon -t test_setpgid_no_t -- selinux_task_setpgid_source 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #2: task_setpgid passed."
		return 0
	else
		echo "Test #2: task_setpgid failed."
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
