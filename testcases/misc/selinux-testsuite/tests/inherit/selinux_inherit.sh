#!/bin/sh

setup()
{
        LTPTMP="/tmp/selinux"
        export TCID="setup"
        export TST_COUNT=0

        # Clean up from a previous run
        rm -f $LTPTMP/test_file 2>&1

	# Create a test file with the test_inherit_file_t type 
	# for use in the tests.
	touch $LTPTMP/test_file
	chcon -t test_inherit_file_t $LTPTMP/test_file

        SAVEPWD=${PWD}
        cd ${LTPROOT}/testcases/bin
        CURRENTDIR="."
}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that test_inherit_nouse_t cannot inherit the rw fd to 
	# the test_file from test_inherit_parent_t.
	# Should fail on fd use permission.

	runcon -t test_inherit_parent_t -- selinux_inherit_parent test_inherit_nouse_t $LTPTMP/test_file selinux_inherit_child 2>&1
	RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #1: inherit passed."
		return 0
        else
                echo "Test #1: inherit failed."
		return 1
        fi
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# Verify that test_inherit_nowrite_t cannot inherit the rw fd 
	# to the test_file from test_inherit_parent_t.
	# Should fail on file write permission.

	runcon -t test_inherit_parent_t -- $CURRENTDIR/selinux_inherit_parent test_inherit_nowrite_t $LTPTMP/test_file $CURRENTDIR/selinux_inherit_child 2>&1
	RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #2: inherit passed."
		return 0
        else
                echo "Test #2: inherit failed."
		return 1
        fi
}

test03()
{

        TCID="test03"
        TST_COUNT=3
        RC=0

	# Verify that test_inherit_child_t can inherit the rw fd to the
	# test file from test_inherit_parent_t.

	runcon -t test_inherit_parent_t -- $CURRENTDIR/selinux_inherit_parent test_inherit_child_t $LTPTMP/test_file $CURRENTDIR/selinux_inherit_child 2>&1
	RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #3: inherit failed."
        else
                echo "Test #3: inherit passed."
	fi
	return $RC
}

cleanup()
{
	# Cleanup.
	rm -rf $LTPTMP/test_file
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
test03 || exit $RC
cleanup
exit 0
