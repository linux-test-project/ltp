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
	export T_COUNT=0 

	# Remove any leftover test file from prior failed runs.
	rm -rf $LTPTMP/test_file

	# Create a test file with the test_stat_file_t type
	# for use in the tests.
	touch $LTPTMP/test_file
	chcon -t test_stat_file_t $LTPTMP/test_file
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_stat_t can get attributes on the file.
	runcon -t test_stat_t -- ls -l $LTPTMP/test_file 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                echo "Test #1: stat passed."
        else
                echo "Test #1: stat failed."
        fi
        return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_nostat_t cannot get attributes on the file.
	runcon -t test_nostat_t -- ls -l $LTPTMP/test_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                echo "Test #2: stat passed."
		return 0
        else
                echo "Test #2: stat failed."
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
cleanup
exit 0
