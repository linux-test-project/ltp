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
	export TST_TOTAL=6

	SELINUXTMPDIR=$(mktemp -d)
	chcon -t test_file_t $SELINUXTMPDIR

	# Create a test directory with the test_addname_dir_t type 
	# for use in the tests.
	mkdir $SELINUXTMPDIR/test_dir 2>&1
	chcon -t test_link_dir_t $SELINUXTMPDIR/test_dir

	# Create a test file with the test_link_file_t type.
	touch $SELINUXTMPDIR/test_dir/test_file
	chcon -t test_link_file_t $SELINUXTMPDIR/test_dir/test_file
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_link_t can create another hard link 
	# to the test file in the test directory.

	runcon -t test_link_t ln $SELINUXTMPDIR/test_dir/test_file $SELINUXTMPDIR/test_dir/test_link 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "link passed."
	else
		tst_resm TFAIL "link failed."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_nolink_t cannot create a hard link
	# to the test file.
	# Should fail on the link permission check.
	runcon -t test_nolink_t ln $SELINUXTMPDIR/test_dir/test_file $SELINUXTMPDIR/test_dir/test_link2 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "link passed."
		RC=0
	else
		tst_resm TFAIL "link failed."
		RC=1
	fi
	return $RC
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	# Verify that test_nolink2_t cannot create a hard link in 
	# the test directory.
	# Should fail on the add_name permission check.
	runcon -t test_nolink2_t ln $SELINUXTMPDIR/test_dir/test_file $SELINUXTMPDIR/test_dir/test_link3 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "link passed."
		RC=0
	else
		tst_resm TFAIL "link failed."
		RC=1
	fi
	return $RC
}

test04()
{
	TCID="test04"
	TST_COUNT=4
	RC=0

	# Verify that test_unlink_t can remove a hard link 
	# to the test file.
	runcon -t test_unlink_t -- rm -f $SELINUXTMPDIR/test_dir/test_link 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "link passed."
	else
		tst_resm TFAIL "link failed."
	fi
	return $RC
}

test05()
{
	TCID="test05"
	TST_COUNT=5
	RC=0

	# Verify that test_nounlink_t cannot remove a 
	# hard link to the test file.
	# Should fail on the unlink permission check.
	runcon -t test_nounlink_t -- rm -f $SELINUXTMPDIR/test_dir/test_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "link passed."
		RC=0
	else
		tst_resm TFAIL "link failed."
		RC=1
	fi
	return $RC
}

test06()
{
	TCID="test06"
	TST_COUNT=6
	RC=0

	# Verify that test_nounlink2_t cannot remove 
	# a hard link in the test directory.
	# Should fail on the remove_name permission check.
	runcon -t test_nounlink2_t -- rm -f $SELINUXTMPDIR/test_dir/test_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "link passed."
		RC=0
	else
		tst_resm TFAIL "link failed."
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
test06 || EXIT_VAL=$RC
cleanup
exit $EXIT_VAL
