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
	export TST_TOTAL=9

	# Remove any leftover test directories from prior failed runs.
	rm -rf $LTPTMP/src_dir $LTPTMP/dst_dir

	# Create the source and destination test directories for the rename.
	mkdir --context=system_u:object_r:test_rename_src_dir_t $LTPTMP/src_dir 2>&1
	mkdir --context=system_u:object_r:test_rename_dst_dir_t $LTPTMP/dst_dir 2>&1

	# Create a test file to try renaming.
	touch $LTPTMP/src_dir/test_file
	chcon -t test_rename_file_t $LTPTMP/src_dir/test_file

	# Create a test directory to try renaming.
	mkdir --context=system_u:object_r:test_rename_dir_t $LTPTMP/src_dir/test_dir

}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_rename_t can rename the test file.
	runcon -t test_rename_t mv $LTPTMP/src_dir/test_file $LTPTMP/dst_dir 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "Test #1: rename passed."
        else
                tst_resm TFAIL "Test #1: rename failed."
        fi
        return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Revert.
	mv $LTPTMP/dst_dir/test_file $LTPTMP/src_dir 2>&1

	# Create a pre-existing destination file.
	touch $LTPTMP/dst_dir/test_file

	# Verify that test_rename2_t can rename the file,
	# removing the pre-existing destination file.
	runcon -t test_rename2_t -- mv -f $LTPTMP/src_dir/test_file $LTPTMP/dst_dir 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "Test #2: rename passed."
        else
                tst_resm TFAIL "Test #2: rename failed."
        fi
        return $RC
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	# Revert.
	mv $LTPTMP/dst_dir/test_file $LTPTMP/src_dir 2>&1

	# Verify that test_rename_t can rename the test dir.
	runcon -t test_rename_t mv $LTPTMP/src_dir/test_dir $LTPTMP/dst_dir 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "Test #3: rename passed."
        else
                tst_resm TFAIL "Test #3: rename failed."
        fi
        return $RC
}

test04()
{
	TCID="test04"
	TST_COUNT=4
	RC=0

	# Revert.
	mv $LTPTMP/dst_dir/test_dir $LTPTMP/src_dir 2>&1

	# Verify that test_norename_t cannot rename the test file.
	# Should fail on the rename permission check to the file.
	runcon -t test_norename_t mv $LTPTMP/src_dir/test_file $LTPTMP/dst_dir 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "Test #4: rename passed."
		RC=0
        else
                tst_resm TFAIL "Test #4: rename failed."
		RC=1
        fi
	return $RC
}

test05()
{
	TCID="test05"
	TST_COUNT=5
	RC=0

	# Verify that test_norename2_t cannot rename the test file.
	# Should fail on the remove_name permission check to the src_dir.
	runcon -t test_norename2_t mv $LTPTMP/src_dir/test_file $LTPTMP/dst_dir 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "Test #5: rename passed."
		RC=0
        else
                tst_resm TFAIL "Test #5: rename failed."
		RC=1
	fi
	return $RC
}

test06()
{
	TCID="test06"
	TST_COUNT=6
	RC=0

	# Verify that test_norename3_t cannot rename the test file.
	# Should fail on the add_name permission check to the dst_dir.
	runcon -t test_norename3_t mv $LTPTMP/src_dir/test_file $LTPTMP/dst_dir 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "Test #6: rename passed."
		RC=0
        else
                tst_resm TFAIL "Test #6: rename failed."
		RC=1
	fi
	return $RC
}

test07()
{
	TCID="test07"
	TST_COUNT=7
	RC=0

	# Create a pre-existing destination file again.
	touch $LTPTMP/dst_dir/test_file

	# Verify that test_norename4_t cannot rename the source file
	# to the destination file.
	# Should fail on the remove_name permission check to the dst_dir.
	runcon -t test_norename4_t -- mv -f $LTPTMP/src_dir/test_file $LTPTMP/dst_dir 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "Test #7: rename passed."
		RC=0
        else
                tst_resm TFAIL "Test #7: rename failed."
		RC=1
	fi
	return $RC
}

test08()
{
	TCID="test08"
	TST_COUNT=8
	RC=0

	# Verify that test_norename5_t cannot rename the source file
	# to the destination file.
	# Should fail on the unlink permission check to the dst_file.
	runcon -t test_norename5_t -- mv -f $LTPTMP/src_dir/test_file $LTPTMP/dst_dir 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "Test #8: rename passed."
		RC=0
        else
                tst_resm TFAIL "Test #8: rename failed."
		RC=1
	fi
	return $RC
}

test09()
{
	TCID="test09"
	TST_COUNT=9
	RC=0

	# Verify that test_norename6_t cannot rename the test dir.
	# Should fail on the reparent check.
	runcon -t test_norename6_t mv $LTPTMP/src_dir/test_dir $LTPTMP/dst_dir 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "Test #9: rename passed."
		RC=0
        else
                tst_resm TFAIL "Test #9: rename failed."
		RC=1
	fi
	return $RC
}

cleanup()
{
	# Cleanup.
	rm -rf $LTPTMP/src_dir $LTPTMP/dst_dir
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
test07 || EXIT_VAL=$RC
test08 || EXIT_VAL=$RC
test09 || EXIT_VAL=$RC
cleanup
exit $EXIT_VAL
