#!/bin/sh

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that notfromdomain cannot transition to todomain.
	# Should fail on the transition permission check.
	runcon -t test_dyntrans_notfromdomain_t -- selinux_dyntrans_parent test_dyntrans_todomain_t 2>&1
	RC=$?
	if [ $RC -ne 0 ]	# we expect this to fail
	then
		echo "Test #1: dyntrans passed."
		return 0
	else
		echo "Test #1: dynstrans failed."
		return 1
	fi
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# Verify that fromdomain can transition to todomain.
	runcon -t test_dyntrans_fromdomain_t -- selinux_dyntrans_parent test_dyntrans_todomain_t 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "Test #2: dyntrans passed."
	else
		echo "Test #2: dynstrans failed."
	fi
	return $RC
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
