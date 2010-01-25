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
        export TCID="setup"
        export TST_COUNT=0
	export TST_TOTAL=4

	SELINUXTMPDIR=$(mktemp -d)
	chcon -t test_file_t $SELINUXTMPDIR

	# Create a test dir with the test_rxdir_dir_t type
	# for use in the tests.
	mkdir $SELINUXTMPDIR/test_dir
	chcon -t test_rxdir_dir_t $SELINUXTMPDIR/test_dir

	# Touch a file in the directory.
	touch $SELINUXTMPDIR/test_dir/test_file
}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that test_rdir_t can read but not search the directory.
	runcon -t test_rdir_t -- ls $SELINUXTMPDIR/test_dir 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "rxdir passed."
        else
                tst_resm TFAIL "rxdir failed."
        fi
        return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	runcon -t test_rdir_t -- ls $SELINUXTMPDIR/test_dir/test_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "rxdir passed."
		RC=0
        else
                tst_resm TFAIL "rxdir failed."
		RC=1
        fi
	return $RC
}

test03()
{
        TCID="test03"
        TST_COUNT=3
        RC=0

	# Verify that test_xdir_t can search but not read the directory.
	runcon -t test_xdir_t -- ls $SELINUXTMPDIR/test_dir/test_file 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "rxdir passed."
        else
                tst_resm TFAIL "rxdir failed."
        fi
	return $RC
}

test04()
{
        TCID="test04"
        TST_COUNT=4
        RC=0

	runcon -t test_xdir_t -- ls $SELINUXTMPDIR/test_dir 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "rxdir passed."
		RC=0
        else
                tst_resm TFAIL "rxdir failed."
		RC=1
        fi
	return $RC
}

cleanup()
{
	rm -rf $SELINUXTMPDIR
}

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, and test functions.
EXIT_VAL=0

setup
test01 || EXIT_VAL=$RC
test02 || EXIT_VAL=$RC
test03 || EXIT_VAL=$RC
test04 || EXIT_VAL=$RC
cleanup
exit $EXIT_VAL 
