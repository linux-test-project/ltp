#!/bin/sh
#
# This test performs capability tests for file operations.
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
	export TST_TOTAL=10

	SELINUXTMPDIR=$(mktemp -d)
	chcon -t test_file_t $SELINUXTMPDIR
}

#
# Tests for the good domains.
#

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# CAP_CHOWN
	touch $SELINUXTMPDIR/temp_file 2>&1
	runcon -t test_fcap_t -- chown daemon $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "capable_file passed."
	else
		tst_resm TFAIL "capable_file failed."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# CAP_FOWNER
	chown daemon.tty $SELINUXTMPDIR/temp_file 2>&1
	runcon -t test_fcap_t -- chmod 0400 $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then 
		tst_resm TPASS "capable_file passed."
	else
		tst_resm TFAIL "capable_file failed."
	fi
	return $RC
}

test03()
{

	TCID="test03"
	TST_COUNT=3
	RC=0

	# CAP_FSETID
	MODE_BEFORE=`stat --format %a $SELINUXTMPDIR/temp_file` 
	runcon -t test_fcap_t -- chmod g+rs $SELINUXTMPDIR/temp_file 2>&1
	MODE_AFTER=`stat --format %a $SELINUXTMPDIR/temp_file`

	# prior mode should not be same as current mode
	if [ $MODE_BEFORE -eq $MODE_AFTER ] 
	then
		tst_resm TFAIL "capable_file failed."
		RC=1
	else
		tst_resm TPASS "capable_file passed."
	fi
	return $RC
}

test04()
{
	TCID="test04"
	TST_COUNT=4
	RC=0

	# CAP_LEASE
	runcon -t test_fcap_t --  selinux_lease $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "capable_file passed."
	else
		tst_resm TFAIL "capable_file failed."
	fi
	return $RC
}

test05()
{

	TCID="test05"
	TST_COUNT=5
	RC=0

	# CAP_MKNOD
	runcon -t test_fcap_t -- mknod $SELINUXTMPDIR/temp_file2 c 5 5 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "capable_file passed."
	else
		tst_resm TFAIL "capable_file failed."
	fi
	return $RC
}

#
# Tests for the bad domain.
#
test06()
{
	TCID="test06"
	TST_COUNT=6
	RC=0

	# CAP_CHOWN
	touch $SELINUXTMPDIR/temp_file 2>&1
	runcon -t test_nofcap_t -- chown daemon $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "capable_file passed."
		RC=0
	else
		tst_resm TFAIL "capable_file failed."
		RC=1
	fi
	return $RC
}
	
test07()
{
	TCID="test07"
	TST_COUNT=7
	RC=0

	# CAP_FOWNER
	chown daemon.tty $SELINUXTMPDIR/temp_file 2>&1
	runcon -t test_nofcap_t -- chmod 0400 $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "capable_file passed."
		RC=0
	else
		tst_resm TFAIL "capable_file failed."
		RC=1
	fi
	return $RC
}

test08()
{
	TCID="test08"
	TST_COUNT=8
	RC=0

	# CAP_FSETID - Domain needs CAP_FOWNER
	MODE_BEFORE=`stat --format %a $SELINUXTMPDIR/temp_file`
	runcon -t test_resfcap_t -- chmod g+rs $SELINUXTMPDIR/temp_file 2>&1

	MODE_AFTER=`stat --format %a $SELINUXTMPDIR/temp_file`
	# prior mode should be same as current mode
	if [ $MODE_BEFORE -eq $MODE_AFTER ] 
	then
		tst_resm TPASS "capable_file passed."
	else
		tst_resm TFAIL "capable_file failed."
	 	RC=1	
	fi 
	return $RC
}

test09()
{
	TCID="test09"
	TST_COUNT=9
	RC=0

	# CAP_LEASE - Needs DAC_OVERRIDE
	runcon -t test_resfcap_t --  selinux_lease $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "capable_file passed."
		RC=0
	else
		tst_resm TFAIL "capable_file failed."
		RC=1
	fi
	return $RC
}

test10()
{
	TCID="test10"
	TST_COUNT=10
	RC=0

	# CAP_MKNOD - Domain needs CAP_DAC_OVERRIDE
	runcon -t test_resfcap_t -- mknod $SELINUXTMPDIR/temp_file2 c 5 5 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "capable_file passed."
		RC=0
	else
		tst_resm TFAIL "capable_file failed."
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
setup
test06 || EXIT_VAL=$RC
test07 || EXIT_VAL=$RC
test08 || EXIT_VAL=$RC
test09 || EXIT_VAL=$RC
test10 || EXIT_VAL=$RC
cleanup
exit $EXIT_VAL
