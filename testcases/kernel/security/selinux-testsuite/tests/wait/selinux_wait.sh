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
	export TST_TOTAL=2

	# run tests in $LTPROOT/testcases/bin directory
        SAVEPWD=${PWD}
        cd ${LTPBIN}
        CURRENTDIR="."
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_wait_parent_t can wait on test_wait_child_t.
	runcon -t test_wait_parent_t -- $CURRENTDIR/selinux_wait_parent test_wait_child_t $CURRENTDIR/selinux_wait_child 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "$TCID   PASS : wait passed."
	else
		echo "$TCID   FAIL : wait failed."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_wait_parent_t cannot wait on test_wait_notchild_t.
	runcon -t test_wait_parent_t -- $CURRENTDIR/selinux_wait_parent test_wait_notchild_t $CURRENTDIR/selinux_wait_child 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "$TCID   PASS : wait passed."
		RC=0
	else
		echo "$TCID   FAIL : wait failed."
		RC=1
	fi
	return $RC
}

cleanup()
{
	# switch back to $LTPROOT directory
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
cleanup
exit $EXIT_VAL
