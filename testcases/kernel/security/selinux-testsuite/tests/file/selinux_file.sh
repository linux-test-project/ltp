#!/bin/sh
#
# This test performs access checks for a file.
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
	export TST_TOTAL=14

	LTPBIN=${LTPBIN:-$LTPROOT/testcases/bin}
	SELINUXTMPDIR=$(mktemp -d)
	chcon -t test_file_t $SELINUXTMPDIR

	if SELINUXTMPDIR=$(mktemp -d); then

		chcon -t test_file_t $SELINUXTMPDIR

		#
		# Create the temp files
		#
		dd if=/dev/zero of=$SELINUXTMPDIR/temp_file count=2 ibs=1024 2>&1 > /dev/null
		dd if=/dev/zero of=$SELINUXTMPDIR/temp_file2 count=2 ibs=1024 2>&1 > /dev/null
		dd if=/dev/zero of=$SELINUXTMPDIR/temp_file3 count=2 ibs=1024 2>&1 > /dev/null
		chmod 775 $SELINUXTMPDIR/temp_file 2>&1 > /dev/null
		chmod 775 $SELINUXTMPDIR/temp_file2 2>&1 > /dev/null

		#
		# Change the context for the file the good domain only has access to.
		#
		chcon -t fileop_file_t $SELINUXTMPDIR/temp_file 2>&1 > /dev/null

		#
		# Change the context for the r/w file for the bad domain
		#
		chcon -t nofileop_rw_file_t $SELINUXTMPDIR/temp_file2 2>&1 > /dev/null

		#
		# Change the context for the read-only access file for the bad domain
		#
		chcon -t nofileop_ra_file_t $SELINUXTMPDIR/temp_file3 2>&1 > /dev/null

		# 
		# Change the context of the test executable
		#
		chcon -t fileop_exec_t $LTPBIN/selinux_wait_io 2>&1 > /dev/null

		#
		# Get the SID of the good file.
		#
		good_file_sid=`ls -Z $SELINUXTMPDIR/temp_file | awk '{print $4}'`

	fi
	# 
	# Change the context of the test executable
	#
	chcon -t fileop_exec_t selinux_wait_io 2>&1 > /dev/null

}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	#
	# Attempt to access a restricted file as the 'good' domain. 
	# The first test hits basic permissions, while the remaining 
	# tests hit specific hooks.
	#
	runcon -t test_fileop_t -- touch $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TFAIL "file failed."
	else
		tst_resm TPASS "file passed."
	fi
	return $RC
}

test02() 
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	runcon -t test_fileop_t -- selinux_seek $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TFAIL "file failed."
        else
		tst_resm TPASS "file passed."
	fi
	return $RC
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	runcon -t test_fileop_t -- selinux_mmap $SELINUXTMPDIR/temp_file $good_file_sid 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TFAIL "file failed."
	else
		tst_resm TPASS "file passed."
	fi
	return $RC
}

test04()
{
	TCID="test04"
	TST_COUNT=4
	RC=0

	runcon -t test_fileop_t -- selinux_mprotect $SELINUXTMPDIR/temp_file $good_file_sid 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TFAIL "file failed."
	else
		tst_resm TPASS "file passed."
	fi
	return $RC
}

test05()
{
	TCID="test05"
	TST_COUNT=5
	RC=0

	runcon -t test_fileop_t -- selinux_lock $SELINUXTMPDIR/temp_file $good_file_sid 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TFAIL "file failed."
	else
		tst_resm TPASS "file passed."
	fi
	return $RC
}

test06()
{
	TCID="test06"
	TST_COUNT=6
	RC=0

	runcon -t test_fileop_t -- selinux_fcntl $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TFAIL "file failed."
	else
		tst_resm TPASS "file passed."
	fi
	return $RC
}

test07()
{
	TCID="test07"
	TST_COUNT=7
	RC=0
	echo -n "" > /dev/tty
	if [ $? -ne 0 ]; then
	    echo "$TCID   INFO : No controlling tty."
	    return $RC
	fi

	#
	# Attempt to create a SIGIO as the 'good' domain. 
	#

	# Run testcase in $LTPROOT/testcases/bin directory
	SAVEPWD=${PWD}
	cd ${0%/*}

	runcon -t test_fileop_t -- selinux_sigiotask 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TFAIL "file failed."
	else
		tst_resm TPASS "file passed."
	fi
	
	# return to $LTPROOT directory
	cd ${SAVEPWD}

	return $RC
}

test08()
{
	TCID="test08"
	TST_COUNT=8
	RC=0

	#
	# Attempt to access the restricted file as the 'bad' domain.
	# The first test hits basic permissions, while the remaining 
	# tests hit specific hooks.
	#
	runcon -t test_nofileop_t -- touch $SELINUXTMPDIR/temp_file 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "file passed."
		RC=0
	else
		tst_resm TFAIL "file failed."
		RC=1
	fi
	return $RC
}

#
# Perform tests for the bad domain.
#

test09()
{
	TCID="test09"
	TST_COUNT=9
	RC=0

	# This seek test will succeed because the llseek hook only verifies
	# that the process has access to the file descriptor. In order to 
	# test llseek properly, a policy change would need to take effect 
	# between the time that the file was opened and the seek took place. 
	# So, for now, we just test the basic access which should succeed.

	runcon -t test_nofileop_t -- selinux_seek $SELINUXTMPDIR/temp_file2 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TFAIL "file failed."
	else
		tst_resm TPASS "file passed."
	fi
	return $RC
}

test10()
{
	TCID="test10"
	TST_COUNT=10
	RC=0

	runcon -t test_nofileop_t -- selinux_mmap $SELINUXTMPDIR/temp_file2 $good_file_sid 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "file passed."
		RC=0
	else
		tst_resm TFAIL "file failed."
		RC=1
	fi
	return $RC
}

test11()
{
	TCID="test11"
	TST_COUNT=11
	RC=0

	chcon -t nofileop_rw_file_t $SELINUXTMPDIR/temp_file2 2>&1 > /dev/null
	runcon -t test_nofileop_t -- selinux_mprotect $SELINUXTMPDIR/temp_file2 $good_file_sid 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "file passed."
		RC=0
	else
		tst_resm TFAIL "file failed."
		RC=1
	fi
	return $RC
}

test12()
{
	TCID="test12"
	TST_COUNT=12
	RC=0

	chcon -t nofileop_rw_file_t $SELINUXTMPDIR/temp_file2 2>&1 > /dev/null
	runcon -t test_nofileop_t -- selinux_lock $SELINUXTMPDIR/temp_file2 $good_file_sid 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "file passed."
		RC=0
	else
		tst_resm TFAIL "file failed."
		RC=1
	fi
	return $RC
}

test13()
{
	TCID="test13"
	TST_COUNT=13
	RC=0

	chcon -t nofileop_rw_file_t $SELINUXTMPDIR/temp_file2 2>&1 > /dev/null

	#
	# Check the fcntl for the bad domain.
	# This uses the read-only accessable file.
	#
	runcon -t test_nofileop_t -- selinux_nofcntl $SELINUXTMPDIR/temp_file3 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TFAIL "file failed."
	else
		tst_resm TPASS "file passed."
	fi
	return $RC
}

test14() 
{
	TCID="test14"
	TST_COUNT=14
	RC=0
	echo -n "" > /dev/tty
	if [ $? -ne 0 ]; then
	    echo "$TCID   INFO : No controlling tty."
	    return $RC
	fi

	#
	# Attempt to create a SIGIO as the 'bad' domain. 
	#
	runcon -t test_nofileop_t -- selinux_sigiotask 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "file passed."
		RC=0
	else
		tst_resm TFAIL "file failed."
		RC=1
	fi
	return $RC
}

cleanup()
{
	rm -rf $SELINUXTMPDIR
}

#
# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, and test functions.
EXIT_VAL=0

if setup ; then
	test01 || EXIT_VAL=$RC
	test02 || EXIT_VAL=$RC
	test03 || EXIT_VAL=$RC
	test04 || EXIT_VAL=$RC
	test05 || EXIT_VAL=$RC
	test06 || EXIT_VAL=$RC
	test07 || EXIT_VAL=$RC
	test08 || EXIT_VAL=$RC
	test09 || EXIT_VAL=$RC
	test10 || EXIT_VAL=$RC
	test11 || EXIT_VAL=$RC
	test12 || EXIT_VAL=$RC
	test13 || EXIT_VAL=$RC
	test14 || EXIT_VAL=$RC
	cleanup
fi
exit $EXIT_VAL 
