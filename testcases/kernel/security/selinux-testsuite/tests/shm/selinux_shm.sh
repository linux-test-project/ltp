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

# ================================================================= #
#  Basic Shared Memory Test Cases                                   #
# ================================================================= #

setup()
{
	export TCID="setup"
	export TST_COUNT=0
	export TST_TOTAL=10
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# First time should just get the resource
	output=`runcon -t test_ipc_base_t selinux_shmget`
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "shm passed."
	else
		tst_resm TFAIL "shm failed."
		return $RC
	fi

	ipcid=`echo $output | grep -o id\ =\ [0-9]*$ | awk '{ print $NF }'`
	if [ ! $ipcid ]
	then
		echo "$TCID: Invalid output from selinux_shmget."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Delete the resource
	runcon -t test_ipc_base_t ipcrm shm $ipcid
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "shm passed."
	else
		tst_resm TFAIL "shm failed."
	fi
	return $RC
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	# Create it again
	output=`runcon -t test_ipc_base_t selinux_shmget`
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "shm passed."
	else
		tst_resm TFAIL "shm failed."
		return $RC
	fi

	ipcid=`echo $output | grep -o id\ =\ [0-9]*$ | awk '{ print $NF }'`
	if [ ! $ipcid ]
	then
		echo "$TCID: Invalid output from selinux_shmget."
	fi
	return $RC
}

test04()
{
	TCID="test04"
	TST_COUNT=4
	RC=0

	# Create it one more time to check associate permission
	output=`runcon -t test_ipc_base_t selinux_shmget`
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "shm passed."
	else
		tst_resm TFAIL "shm failed."
		return $RC
	fi

	ipcid2=`echo $output | grep -o id\ =\ [0-9]*$ | awk '{ print $NF }'`
	if [ ! $ipcid2 ]
	then
		echo "$TCID: Invalid output from selinux_shmget."
	fi
	return $RC
}

test05()
{
	TCID="test05"
	TST_COUNT=5
	RC=0

	# Make sure they match
	if [ $ipcid2 = $ipcid ]
	then
		tst_resm TPASS "shm passed."
		RC=0
	else
		tst_resm TFAIL "shm failed."
		RC=1
	fi
	return $RC
}

test06()
{
	TCID="test06"
	TST_COUNT=6
	RC=0

	# Try to associate with it from the unprivileged domain
	runcon -t test_ipc_none_t selinux_shmget
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "shm passed."
		RC=0
	else
		tst_resm TFAIL "shm failed."
		RC=1	
	fi
	return $RC
}

# ================================================================= #
# Shared Memory Control Tests (shmctl syscall)                      #
# ================================================================= #

test07()
{
	TCID="test07"
	TST_COUNT=7
	RC=0

	# run shared memory control tests, we expect all to fail
	output=`runcon -t test_ipc_associate_t selinux_shmctl`
        number=`echo $output | awk '{ print NF }'`         
	result=`echo $output | awk '{s = 0; n=split($0,a," "); for (i=1; i <= NF; i++) s += a[i]; print s }'`

        # The results should be a row of -1's separated by a space.
        # Like "-1 -1 -1". The only way I know to check this from shell
        # is to add up the fields and see if they equal -<number of fields".
        # For example, "-1 -1 -1" is -3.  That would indicate all fields
        # were a "-1".
        # Change this if there is a better way to do this check in shell.

        if [ $result = "-$number" ]
        then
                tst_resm TPASS "shm passed."
                RC=0
        else
                tst_resm TFAIL "shm failed."
                RC=1
        fi
	return $RC
}

test08()
{
	TCID="test08"
	TST_COUNT=8
	RC=0

	# run shared memory control tests, we expect all to succeed
	# last test should delete the object
	output=`runcon -t test_ipc_base_t selinux_shmctl`
	result=`echo $output | awk '{s = 0; n=split($0,a," "); for (i=1; i <= NF; i++) s += a[i]; print s }'`

        # The results should be a row of 0's. Like "0 0 0". The only way
        # I know to check this from shell is to add up the fields and
        # see if they are equal to zero. For example "0 0 0" is 0. If a
        # Change this if there is a better way to do this check in shell.
        if [ $result = 0 ]
        then
                tst_resm TPASS "shm passed."
                RC=0
        else
                tst_resm TFAIL "shm failed."
                RC=1
        fi
	return $RC
}

# ================================================================= #
# Shared Memory Operation Tests (shmat)                             #
# ================================================================= #

test09()
{
	TCID="test09"
	TST_COUNT=9
	RC=0

	runcon -t test_ipc_base_t selinux_shmat
	
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "shm passed."
	else
		tst_resm TFAIL "shm failed."
	fi
	return $RC
}

test10()
{
	TCID="test10"
	TST_COUNT=10
	RC=0

	runcon -t test_ipc_associate_t selinux_shmat
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "shm passed."
		RC=0
	else
		tst_resm TFAIL "shm failed."
		RC=1
	fi
	return $RC
}
	

cleanup()
{
	# Cleanup 
	output=`runcon -t test_ipc_base_t selinux_shmget`
	ipcid=`echo $output | grep -o id\ =\ [0-9]*$ | awk '{ print $NF }'`
	if [ ! $ipcid ]
	then
		echo "cleanup: Invalid output from selinux_shmget."
	fi
	runcon -t test_ipc_base_t ipcrm shm $ipcid
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
test10 || EXIT_VAL=$RC
cleanup
exit $EXIT_VAL
