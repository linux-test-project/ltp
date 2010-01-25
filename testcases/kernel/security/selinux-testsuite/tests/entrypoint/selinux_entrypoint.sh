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

	SELINUXTMPDIR=$(mktemp -d)
	chcon -t test_file_t $SELINUXTMPDIR
}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that test_entrypoint_t cannot be entered 
	# via an ordinary program.
	runcon -t test_entrypoint_t true 2>&1
	RC=$?   # this should fail
        if [ $RC -ne 0 ]
        then
		tst_resm TPASS "entrypoint passed."
		RC=0
	else
		tst_resm TFAIL "entrypoint failed."
		RC=1
	fi
	return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# Set up a program with the entrypoint type for this domain.
	cp /bin/true $SELINUXTMPDIR/true
	chcon -t test_entrypoint_execute_t $SELINUXTMPDIR/true

	# Verify that test_entrypoint_t can be entered via this program.
	runcon -t test_entrypoint_t $SELINUXTMPDIR/true
	RC=$?
        if [ $RC -ne 0 ]
        then
		tst_resm TFAIL "entrypoint failed."
	else
		tst_resm TPASS "entrypoint passed."
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
cleanup
exit $EXIT_VAL 
