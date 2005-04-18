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

	# Remove any leftover test file from prior failed runs.
	rm -rf $LTPTMP/test_file $LTPTMP/test_file2 $LTPTMP/test_sock

	# Create and label the test files.
	touch $LTPTMP/test_file $LTPTMP/test_file2
	chcon -t test_fdreceive_file_t $LTPTMP/test_file
	chcon -t test_fdreceive_file2_t $LTPTMP/test_file2

	# Start server process in test_fdreceive_server_t.
	runcon -t test_fdreceive_server_t selinux_fdreceive_server $LTPTMP/test_sock &
	PID=$!
	sleep 1; # Give it a moment to initialize.
}

test01()
{

	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_fdreceive_server_t can receive a rw fd to
	# the test_file from test_fdreceive_client_t.
	runcon -t test_fdreceive_client_t -- selinux_fdreceive_client $LTPTMP/test_file $LTPTMP/test_sock
	RC=$?
	if [ $RC -eq 0 ]
	then
		echo "Test #1: fdreceive passed."
	else
		echo "Test #1: fdreceive failed."
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_fdreceive_server_t cannot receive
	# a rw fd to test_file2. Should fail on file permissions 
	# to test_file2.

	runcon -t test_fdreceive_client_t -- selinux_fdreceive_client $LTPTMP/test_file2 $LTPTMP/test_sock
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #2: fdreceive passed."
		RC=0
	else
		echo "Test #2: fdreceive failed."
		RC=1
	fi
	return $RC
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	# Verify that test_fdreceive_server_t cannot receive
	# a fd created by test_fdreceive_client2_t.
	# Should fail on fd use permission.

	runcon -t test_fdreceive_client2_t -- selinux_fdreceive_client $LTPTMP/test_file $LTPTMP/test_sock
	RC=$?
	if [ $RC -ne 0 ]
	then
		echo "Test #3: fdreceive passed."
		RC=0
	else
		echo "Test #3: fdreceive failed."
		RC=1
	fi
	return $RC
}

cleanup()
{
	# Kill the server.
	kill -s TERM $PID

	# Cleanup.
	rm -rf $LTPTMP/test_file $LTPTMP/test_file2 $LTPTMP/test_sock
}

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, and test functions.

setup 
test01 || exit $RC
test02 || exit $RC
test03 || exit $RC
cleanup
exit 0
