#!/bin/sh

setup()
{
        LTPTMP="/tmp/selinux"
        export TCID="setup"
        export TST_COUNT=0

	# Remove any leftover test directories from prior failed runs.
	rm -rf $LTPTMP/test_file

	# Create a test file.
	touch $LTPTMP/test_file
	chcon -t test_open_file_t $LTPTMP/test_file
}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that test_open_t can open the file for reading and writing.
	runcon -t test_open_t selinux_fopen $LTPTMP/test_file r+ 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                echo "Test #1: open passed."
        else
                echo "Test #1: open failed."
        fi
        return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# Verify that test_noopen_t cannot open the
	# file for reading or writing.
	runcon -t test_noopen_t selinux_fopen $LTPTMP/test_file r 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #2: open passed."
		return 0
        else
                echo "Test #2: open failed."
		return 1
        fi
}

test03()
{
        TCID="test03"
        TST_COUNT=3
        RC=0
	runcon -t test_noopen_t selinux_fopen $LTPTMP/test_file w 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #3: open passed."
		return 0
        else
                echo "Test #3: open failed."
		return 1
        fi
}

test04()
{
        TCID="test04"
        TST_COUNT=4
        RC=0

	runcon -t test_noopen_t selinux_fopen $LTPTMP/test_file r+ 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #4: open passed."
		return 0
        else
                echo "Test #4: open failed."
		return 1
        fi
}

test05()
{

        TCID="test05"
        TST_COUNT=5
        RC=0

	# Verify that test_append_t cannot open the file for writing.
	runcon -t test_append_t selinux_fopen $LTPTMP/test_file w 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #5: open passed."
		return 0
        else
                echo "Test #5: open failed."
		return 1
        fi
}

test06()
{
        TCID="test06"
        TST_COUNT=6
        RC=0

	# Verify that test_append_t can open the file for appending.
	runcon -t test_append_t selinux_fopen $LTPTMP/test_file a 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                echo "Test #6: open passed."
        else
                echo "Test #6: open failed."
        fi
	return $RC
}

test07()
{
        TCID="test07"
        TST_COUNT=7
        RC=0

	# Verify that test_append_t cannot open the file 
	# for appending and then clear the o_append flag.
	runcon -t test_append_t selinux_append2write $LTPTMP/test_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #7: open passed."
		return 0
        else
                echo "Test #7: open failed."
		return 1
        fi
}

test08()
{
        TCID="test08"
        TST_COUNT=8
        RC=0

	# Verify that test_open_t can open the file for appending
	# and then clear the o_append flag.
	runcon -t test_open_t selinux_append2write $LTPTMP/test_file 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                echo "Test #8: open passed."
        else
                echo "Test #8: open failed."
        fi
	return $RC
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
test04 || exit $RC
test05 || exit $RC
test06 || exit $RC
test07 || exit $RC
test08 || exit $RC
cleanup
exit 0
