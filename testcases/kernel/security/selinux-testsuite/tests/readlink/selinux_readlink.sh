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

	# Create a test file.  
	touch $SELINUXTMPDIR/test_file 2>&1
	chcon -t test_readlink_file_t $SELINUXTMPDIR/test_file 2>&1

	# Create a test symbolic link to the test file.
	ln -sf test_file $SELINUXTMPDIR/test_symlink 2>&1
	chcon -h -t test_readlink_link_t $SELINUXTMPDIR/test_symlink 2>&1
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_readlink_t can read and follow this link.
	runcon -t test_readlink_t -- ls -Ll $SELINUXTMPDIR/test_symlink
	RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "readlink passed."
        else
                tst_resm TFAIL "readlink failed."
        fi
        return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_noreadlink_t cannot read or follow this link.
	runcon -t test_noreadlink_t -- ls -l $SELINUXTMPDIR/test_symlink 2>&1
	RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "readlink passed."
		RC=0
        else
                tst_resm TFAIL "readlink failed."
		RC=1
        fi
	return $RC
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	runcon -t test_noreadlink_t -- ls -Ll $SELINUXTMPDIR/test_symlink 2>&1
	RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "readlink passed."
		RC=0
        else
                tst_resm TFAIL "readlink failed."
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
