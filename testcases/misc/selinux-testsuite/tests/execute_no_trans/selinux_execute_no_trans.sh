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

        # Clean up from a previous run
        rm -f $LTPTMP/true 2>&1
}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Set up a program with the denied type for this domain.
	cp /bin/true $LTPTMP/true
	chcon -t test_execute_notrans_denied_t $LTPTMP/true

	# Verify that test_execute_notrans_t cannot execute the denied type.
	runcon -t test_execute_notrans_t -- bash -c $LTPTMP/true 2>&1
	RC=$?		# this should fail
	if [ $RC -ne 0 ]
	then
		echo "Test #1: execute_no_trans passed."
		RC=0
        else
                echo "Test #1: execute_no_trans failed."
		RC=1
        fi
	return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# Set up a program with the allowed type for this domain.
	chcon -t test_execute_notrans_allowed_t $LTPTMP/true

	# Verify that test_execute_notrans_t can execute the allowed type.
	runcon -t test_execute_notrans_t -- bash -c $LTPTMP/true 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #2: execute_no_trans failed."
        else
                echo "Test #2: execute_no_trans passed."
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

setup 
test01 || exit $RC
test02 || exit $RC
cleanup
exit 0
