#!/bin/sh

setup()
{
        LTPTMP="/tmp/selinux"
        export TCID="setup"
        export TST_COUNT=0

        # Clean up from a previous run
        rm -f $LTPTMP/true 2>&1
}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that test_entrypoint_t cannot be entered 
	# via an ordinary program.
	runcon -t test_entrypoint_t $LTPTMP/true 2>&1
	RC=$?   # this should fail
        if [ $RC -ne 0 ]
        then
		echo "Test #1: entrypoint passed."
		return 0
	else
		echo "Test #1: entrypoint failed."
		return 1
	fi
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# Set up a program with the entrypoint type for this domain.
	cp /bin/true $LTPTMP/true
	chcon -t test_entrypoint_execute_t $LTPTMP/true

	# Verify that test_entrypoint_t can be entered via this program.
	runcon -t test_entrypoint_t $LTPTMP/true
        if [ $RC -ne 0 ]
        then
		echo "Test #2: entrypoint failed."
	else
		echo "Test #2: entrypoint passed."
	fi
	return $RC
}

cleanup()
{
	# Cleanup.
	rm -f $LTPTMP/true
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
