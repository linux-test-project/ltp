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
	export TST_TOTAL=3

	SELINUXTMPDIR=$(mktemp -d)
	chcon -t test_file_t $SELINUXTMPDIR

	# Create a test file with the test_relabel_oldtype_t
	# type for use in the tests.
	touch $SELINUXTMPDIR/test_file
	chcon -t test_relabel_oldtype_t $SELINUXTMPDIR/test_file
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_relabel_t can relabel the file.
	runcon -t test_relabel_t -- chcon -t test_relabel_newtype_t $SELINUXTMPDIR/test_file 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "relabel passed."
        else
                tst_resm TFAIL "relabel failed."
        fi
        return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Revert.
	chcon -t test_relabel_oldtype_t $SELINUXTMPDIR/test_file

	# Verify that test_norelabelfrom_t cannot relabel the file.
	# Should fail on the relabelfrom permission check to the original type.
	runcon -t test_norelabelfrom_t -- chcon -t test_relabel_newtype_t $SELINUXTMPDIR/test_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "relabel passed."
		RC=0
        else
                tst_resm TFAIL "relabel failed."
		RC=1
        fi
	return $RC
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	# Verify that test_norelabelto_t cannot relabel
	# the file to the new type.
	# Should fail on the relabelto permission check to the new type.
	runcon -t test_norelabelto_t -- chcon -t test_relabel_newtype_t $SELINUXTMPDIR/test_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "relabel passed."
		RC=0
        else
                tst_resm TFAIL "relabel failed."
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
cleanup
exit $EXIT_VAL
