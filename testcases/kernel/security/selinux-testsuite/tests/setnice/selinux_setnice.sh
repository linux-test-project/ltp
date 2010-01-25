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

	# Verify that test_setnice_change_t can have nice 
	# set by test_setnice_set_t
	runcon -t test_setnice_change_t $CURRENTDIR/selinux_setnice_parent test_setnice_set_t $CURRENTDIR/selinux_setnice_child 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
		tst_resm TPASS "setnice passed."
        else
		tst_resm TFAIL "setnice failed."
        fi
        return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0
	
	# Verify that test_setnice_change_t cannot
	# have nice set by test_setnice_noset_t
	# Should fail on setsched permission.
	runcon -t test_setnice_change_t $CURRENTDIR/selinux_setnice_parent test_setnice_noset_t $CURRENTDIR/selinux_setnice_child 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
		tst_resm TPASS "setnice passed."
		RC=0
        else
                tst_resm TFAIL "setnice failed."
		RC=1
        fi
	return $RC
}

cleanup()
{
	# return to $LTPROOT directory
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
