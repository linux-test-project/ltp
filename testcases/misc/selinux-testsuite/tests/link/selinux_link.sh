#!/bin/sh

setup()
{
	LTPTMP="/tmp/selinux"
	export TCID="setup"
	export TST_COUNT=0

	# Clean up from a previous run
	rm -f $LTPTMP/test_dir 2>&1

	# Create a test directory with the test_addname_dir_t type 
	# for use in the tests.
	mkdir $LTPTMP/test_dir 2>&1
	chcon -t test_link_dir_t $LTPTMP/test_dir

	# Create a test file with the test_link_file_t type.
	touch $LTPTMP/test_dir/test_file
	chcon -t test_link_file_t $LTPTMP/test_dir/test_file
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_link_t can create another hard link 
	# to the test file in the test directory.

	runcon -t test_link_t ln $LTPTMP/test_dir/test_file $LTPTMP/test_dir/test_link 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "Test #1: link passed."
	else
		echo "Test #1: link failed."
	fi
	return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# Verify that test_nolink_t cannot create a hard link
	# to the test file.
	# Should fail on the link permission check.
	runcon -t test_nolink_t ln $LTPTMP/test_dir/test_file $LTPTMP/test_dir/test_link2 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #2: link passed."
		return 0
	else
		echo "Test #2: link failed."
		return 1
	fi
}

test03()
{
        TCID="test03"
        TST_COUNT=3
        RC=0

	# Verify that test_nolink2_t cannot create a hard link in 
	# the test directory.
	# Should fail on the add_name permission check.
	runcon -t test_nolink2_t ln $LTPTMP/test_dir/test_file $LTPTMP/test_dir/test_link3 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #3: link passed."
		return 0
	else
		echo "Test #3: link failed."
		return 1
	fi
}

test04()
{
        TCID="test04"
        TST_COUNT=4
        RC=0

	# Verify that test_unlink_t can remove a hard link 
	# to the test file.
	runcon -t test_unlink_t -- rm -f $LTPTMP/test_dir/test_link 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "Test #4: link passed."
	else
		echo "Test #4: link failed."
	fi
	return $RC
}

test05()
{
        TCID="test05"
        TST_COUNT=5
        RC=0

	# Verify that test_nounlink_t cannot remove a 
	# hard link to the test file.
	# Should fail on the unlink permission check.
	runcon -t test_nounlink_t -- rm -f $LTPTMP/test_dir/test_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #5: link passed."
		return 0
	else
		echo "Test #5: link failed."
		return 1
	fi
}

test06()
{
        TCID="test06"
        TST_COUNT=6
        RC=0

	# Verify that test_nounlink2_t cannot remove 
	# a hard link in the test directory.
	# Should fail on the remove_name permission check.
	runcon -t test_nounlink2_t -- rm -f $LTPTMP/test_dir/test_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #6: link passed."
		return 0
	else
		echo "Test #6: link failed."
		return 1
	fi
}

cleanup()
{
	# Cleanup.
	rm -rf $LTPTMP/test_dir
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
test04 || exit $RC
test05 || exit $RC
test06 || exit $RC
cleanup
exit 0
