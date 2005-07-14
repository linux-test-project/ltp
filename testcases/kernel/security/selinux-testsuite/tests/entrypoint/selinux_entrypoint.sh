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
	export TST_TOTAL=2

        # Clean up from a previous run
        rm -f $LTPTMP/true 2>&1
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
		tst_resm TPASS "Test #1: entrypoint passed."
		RC=0
	else
		tst_resm TFAIL "Test #1: entrypoint failed."
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
	cp /bin/true $LTPTMP/true
	chcon -t test_entrypoint_execute_t $LTPTMP/true

	# Verify that test_entrypoint_t can be entered via this program.
	runcon -t test_entrypoint_t $LTPTMP/true
        if [ $RC -ne 0 ]
        then
		tst_resm TFAIL "Test #2: entrypoint failed."
	else
		tst_resm TPASS "Test #2: entrypoint passed."
	fi
	return $RC
}

cleanup()
{
	# Cleanup.
	rm -f $LTPTMP/true
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
