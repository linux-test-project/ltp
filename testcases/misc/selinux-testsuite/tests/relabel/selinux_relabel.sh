#!/bin/sh

setup()
{
        LTPTMP="/tmp/selinux"
        export TCID="setup"
	export TST_COUNT=0

	# Remove any leftover test file from prior failed runs.
	rm -rf $LTPTMP/test_file

	# Create a test file with the test_relabel_oldtype_t
	# type for use in the tests.
	touch $LTPTMP/test_file
	chcon -t test_relabel_oldtype_t $LTPTMP/test_file
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_relabel_t can relabel the file.
	runcon -t test_relabel_t chcon system_u:object_r:test_relabel_newtype_t $LTPTMP/test_file 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                echo "Test #1: relabel passed."
        else
                echo "Test #1: relabel failed."
        fi
        return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Revert.
	chcon -t test_relabel_oldtype_t $LTPTMP/test_file

	# Verify that test_norelabelfrom_t cannot relabel the file.
	# Should fail on the relabelfrom permission check to the original type.
	runcon -t test_norelabelfrom_t -- chcon -t test_relabel_newtype_t $LTPTMP/test_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #2: relabel passed."
		return 0
        else
                echo "Test #2: relabel failed."
		return 1
        fi
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	# Verify that test_norelabelto_t cannot relabel
	# the file to the new type.
	# Should fail on the relabelto permission check to the new type.
	runcon -t test_norelabelto_t -- chcon -t test_relabel_newtype_t $LTPTMP/test_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #3: relabel passed."
		return 0
        else
                echo "Test #3: relabel failed."
		return 1
        fi
}

cleanup()
{
	# Cleanup.
	rm -rf $LTPTMP/test_file
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
