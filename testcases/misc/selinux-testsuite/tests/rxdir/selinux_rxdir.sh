#!/bin/sh
#
# Copyright (c) 2002 Network Associates Technology, Inc.
# Copyright (c) International Business Machines  Corp., 2005
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#

setup()
{
        LTPTMP="/tmp/selinux"
        export TCID="setup"
        export TST_COUNT=0

	# Remove any leftover test directory from prior failed runs.
	rm -rf $LTPTMP/test_dir

	# Create a test dir with the test_rxdir_dir_t type
	# for use in the tests.
	mkdir --context=system_u:object_r:test_rxdir_dir_t $LTPTMP/test_dir

	# Touch a file in the directory.
	touch $LTPTMP/test_dir/test_file
}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that test_rdir_t can read but not search the directory.
	runcon -t test_rdir_t -- ls $LTPTMP/test_dir 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                echo "Test #1: rxdir passed."
        else
                echo "Test #1: rxdir failed."
        fi
        return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	runcon -t test_rdir_t -- ls $LTPTMP/test_dir/test_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #2: rxdir passed."
		return 0
        else
                echo "Test #2: rxdir failed."
		return 1
        fi
}

test03()
{
        TCID="test03"
        TST_COUNT=3
        RC=0

	# Verify that test_xdir_t can search but not read the directory.
	runcon -t test_xdir_t -- ls $LTPTMP/test_dir/test_file 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                echo "Test #3: rxdir passed."
        else
                echo "Test #3: rxdir failed."
        fi
	return $RC
}

test04()
{
        TCID="test04"
        TST_COUNT=4
        RC=0

	runcon -t test_xdir_t -- ls $LTPTMP/test_dir 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #4: rxdir passed."
		return 0
        else
                echo "Test #4: rxdir failed."
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
cleanup
exit 0
