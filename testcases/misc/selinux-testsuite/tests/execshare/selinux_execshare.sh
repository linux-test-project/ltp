#!/bin/sh

setup()
{
        export TCID="setup"
        export TST_COUNT=0
	SAVEPWD=${PWD}
	cd ${LTPROOT}/testcases/bin
	CURRENTDIR="."
}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that test_execshare_parent_t can share state across
	# an exec with test_execshare_child_t.
	runcon -t test_execshare_parent_t -- $CURRENTDIR/selinux_execshare_parent 0x200 test_execshare_child_t $CURRENTDIR/selinux_execshare_child 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
                echo "Test #1: execshare failed."
        else
                echo "Test #1: execshare passed."
        fi
        return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# Verify that test_execshare_parent_t cannot share state across 
	# an exec with test_execshare_notchild_t.

	runcon -t test_execshare_parent_t -- $CURRENTDIR/selinux_execshare_parent 0x200 test_execshare_notchild_t $CURRENTDIR/selinux_execshare_child 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
                echo "Test #2: execshare passed."
		return 0
        else
                echo "Test #2: execshare failed."
		return 1
        fi
}

cleanup()
{
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
setup
test01 || exit $RC
test02 || exit $RC
cleanup
exit 0
