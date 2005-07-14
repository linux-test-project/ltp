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
	export TST_TOTAL=3

	# Clean up from a previous run
	rm -f $LTPTMP/test_file 2>&1

	# Create a test file with the test_inherit_file_t type 
	# for use in the tests.
	touch $LTPTMP/test_file
	chcon -t test_inherit_file_t $LTPTMP/test_file

	# run tests in $LTPROOT/testcases/bin directory
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
		tst_resm TPASS "Test #1: inherit passed."
		RC=0
	else
		tst_resm TFAIL "Test #1: inherit failed."
		RC=1
	fi
	return $RC
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
		tst_resm TPASS "Test #2: inherit passed."
		RC=0
	else
		tst_resm TFAIL "Test #2: inherit failed."
		RC=1
	fi
	return $RC
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
		tst_resm TFAIL "Test #3: inherit failed."
	else
		tst_resm TPASS "Test #3: inherit passed."
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
EXIT_VAL=0

setup 
test01 || EXIT_VAL=$RC
test02 || EXIT_VAL=$RC
test03 || EXIT_VAL=$RC
cleanup
exit $EXIT_VAL
