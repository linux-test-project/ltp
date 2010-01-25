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
	export TST_TOTAL=5

	SELINUXTMPDIR=$(mktemp -d)
	chcon -t test_file_t $SELINUXTMPDIR

	# Create a test directory with the test_mkdir_dir_t type 
	# for use in the tests.
	mkdir $SELINUXTMPDIR/test_dir 2>&1
	chcon -t test_mkdir_dir_t $SELINUXTMPDIR/test_dir
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_addname_t can create a subdirectory.
	runcon -t test_addname_t mkdir $SELINUXTMPDIR/test_dir/test1 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "mkdir passed."
	else
		tst_resm TFAIL "mkdir failed."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_noaddname_t cannot create a subdirectory.
	# Should fail on the add_name permission check to the test directory.
	runcon -t test_noaddname_t mkdir $SELINUXTMPDIR/test_dir/test2 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "mkdir passed."
		RC=0
	else
		tst_resm TFAIL "mkdir failed."
		RC=1
	fi
	return $RC
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	# Verify that test_nosearch_t cannot create a subdirectory.
	# Should fail on the search permission check to the test directory.
	runcon -t test_nosearch_t mkdir $SELINUXTMPDIR/test_dir/test2 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "mkdir passed."
		RC=0
	else
		tst_resm TFAIL "mkdir failed."
		RC=1
	fi
	return $RC
}

test04()
{
	TCID="test04"
	TST_COUNT=4
	RC=0
	SUFFIX=""
	MLS=x`cat /selinux/mls`
	if [ "$MLS" == "x1" ]
	then
	    SUFFIX=":s0"
	fi

	# Verify that test_create_t can create a subdirectory
	# with a different type.
	# This requires add_name to test_mkdir_dir_t and create
	# to test_create_dir_t.
	runcon -t test_create_t -- mkdir --context=system_u:object_r:test_create_dir_t$SUFFIX $SELINUXTMPDIR/test_dir/test3 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "mkdir passed."
	else
		tst_resm TFAIL "mkdir failed."
	fi
	return $RC
}

test05()
{
	TCID="test05"
	TST_COUNT=5
	RC=0
	SUFFIX=""
	MLS=x`cat /selinux/mls`
	if [ "$MLS" == "x1" ]
	then
	    SUFFIX=":s0"
	fi

	# Verify that test_nocreate_t cannot create 
	# a subdirectory with a different type.
	# Should fail on create check to the new type.
	runcon -t test_nocreate_t -- mkdir --context=system_u:object_r:test_create_dir_t$SUFFIX $SELINUXTMPDIR/test_dir/test4 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "mkdir passed."
		RC=0
	else
		tst_resm TFAIL "mkdir failed."
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
test05 || EXIT_VAL=$RC
cleanup
exit $EXIT_VAL 
