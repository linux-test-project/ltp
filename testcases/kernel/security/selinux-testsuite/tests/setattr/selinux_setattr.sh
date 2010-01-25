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

	# Create a test file with the test_setattr_file_t type
	# for use in the tests.
	touch $SELINUXTMPDIR/test_file
	chcon -t test_setattr_file_t $SELINUXTMPDIR/test_file
}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that test_setattr_t can set attributes on the file.
	runcon -t test_setattr_t chown root $SELINUXTMPDIR/test_file 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "setattr passed."
        else
                tst_resm TFAIL "setattr failed."
        fi
        return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	runcon -t test_setattr_t chmod 0755 $SELINUXTMPDIR/test_file 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "setattr passed."
        else
                tst_resm TFAIL "setattr failed."
        fi
        return $RC
}

test03()
{
        TCID="test03"
        TST_COUNT=3
        RC=0

	# Verify that test_nosetattr_t cannot set attributes on the file.
	runcon -t test_nosetattr_t chown nobody $SELINUXTMPDIR/test_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "setattr passed."
		RC=0
        else
                tst_resm TFAIL "setattr failed."
		RC=1
        fi
	return $RC
}

test04()
{
        TCID="test04"
        TST_COUNT=4
        RC=0

	runcon -t test_nosetattr_t chmod 0644 $SELINUXTMPDIR/test_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "setattr passed."
		RC=0
        else
                tst_resm TFAIL "setattr failed."
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
