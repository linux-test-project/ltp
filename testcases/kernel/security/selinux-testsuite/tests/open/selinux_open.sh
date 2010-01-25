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
	export TST_TOTAL=8

	SELINUXTMPDIR=$(mktemp -d)
	chcon -t test_file_t $SELINUXTMPDIR

	# Create a test file.
	touch $SELINUXTMPDIR/test_file
	chcon -t test_open_file_t $SELINUXTMPDIR/test_file
}

test01()
{
        TCID="test01"
        TST_COUNT=1
        RC=0

	# Verify that test_open_t can open the file for reading and writing.
	runcon -t test_open_t selinux_fopen $SELINUXTMPDIR/test_file r+ 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "open passed."
        else
                tst_resm TFAIL "open failed."
        fi
        return $RC
}

test02()
{
        TCID="test02"
        TST_COUNT=2
        RC=0

	# Verify that test_noopen_t cannot open the
	# file for reading or writing.
	runcon -t test_noopen_t selinux_fopen $SELINUXTMPDIR/test_file r 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "open passed."
		RC=0
        else
                tst_resm TFAIL "open failed."
		RC=1
        fi
	return $RC
}

test03()
{
        TCID="test03"
        TST_COUNT=3
        RC=0
	runcon -t test_noopen_t selinux_fopen $SELINUXTMPDIR/test_file w 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "open passed."
		RC=0
        else
                tst_resm TFAIL "open failed."
		RC=1
        fi
	return $RC
}

test04()
{
        TCID="test04"
        TST_COUNT=4
        RC=0

	runcon -t test_noopen_t selinux_fopen $SELINUXTMPDIR/test_file r+ 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "open passed."
		RC=0
        else
                tst_resm TFAIL "open failed."
		RC=1
        fi
	return $RC
}

test05()
{

        TCID="test05"
        TST_COUNT=5
        RC=0

	# Verify that test_append_t cannot open the file for writing.
	runcon -t test_append_t selinux_fopen $SELINUXTMPDIR/test_file w 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "open passed."
		RC=0
        else
                tst_resm TFAIL "open failed."
		RC=1
        fi
	return $RC
}

test06()
{
        TCID="test06"
        TST_COUNT=6
        RC=0

	# Verify that test_append_t can open the file for appending.
	runcon -t test_append_t selinux_fopen $SELINUXTMPDIR/test_file a 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "open passed."
        else
                tst_resm TFAIL "open failed."
        fi
	return $RC
}

test07()
{
        TCID="test07"
        TST_COUNT=7
        RC=0

	# Verify that test_append_t cannot open the file 
	# for appending and then clear the o_append flag.
	runcon -t test_append_t selinux_append2write $SELINUXTMPDIR/test_file 2>&1
        RC=$?
        if [ $RC -ne 0 ]
        then
                tst_resm TPASS "open passed."
		RC=0
        else
                tst_resm TFAIL "open failed."
		RC=1
        fi
	return $RC
}

test08()
{
        TCID="test08"
        TST_COUNT=8
        RC=0

	# Verify that test_open_t can open the file for appending
	# and then clear the o_append flag.
	runcon -t test_open_t selinux_append2write $SELINUXTMPDIR/test_file 2>&1
        RC=$?
        if [ $RC -eq 0 ]
        then
                tst_resm TPASS "open passed."
        else
                tst_resm TFAIL "open failed."
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
test07 || EXIT_VAL=$RC
test08 || EXIT_VAL=$RC
cleanup
exit $EXIT_VAL
