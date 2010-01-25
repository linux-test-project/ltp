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
	export TST_TOTAL=16

	# Start the process that will receive the signal.
	runcon -t test_kill_server_t selinux_sigkill_server &
	PID=$!

	sleep 1; # Give it a moment to initialize.
}

test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# Verify that test_kill_signal_t cannot send CHLD to the server.
	runcon -t test_kill_signal_t -- kill -s CHLD $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

	# Verify that test_kill_signal_t cannot send STOP to the server.
	runcon -t test_kill_signal_t -- kill -s STOP $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	# Verify that test_kill_signal_t cannot send KILL to the server.
	runcon -t test_kill_signal_t -- kill -s KILL $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test04()
{
	TCID="test04"
	TST_COUNT=4
	RC=0

	# Verify that test_kill_sigchld_t cannot send TERM to the server.
	runcon -t test_kill_sigchld_t -- kill -s TERM $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test05()
{
	TCID="test05"
	TST_COUNT=5
	RC=0

	# Verify that test_kill_sigchld_t cannot send STOP to the server.
	runcon -t test_kill_sigchld_t -- kill -s STOP $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test06()
{
	TCID="test06"
	TST_COUNT=6
	RC=0

	# Verify that test_kill_sigchld_t cannot send KILL to the server.
	runcon -t test_kill_sigchld_t -- kill -s KILL $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test07()
{
	TCID="test07"
	TST_COUNT=7
	RC=0

	# Verify that test_kill_sigstop_t cannot send TERM to the server.
	runcon -t test_kill_sigstop_t -- kill -s TERM $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test08()
{
	TCID="test08"
	TST_COUNT=8
	RC=0

	# Verify that test_kill_sigstop_t cannot send CHLD to the server.
	runcon -t test_kill_sigstop_t -- kill -s CHLD $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test09()
{
	TCID="test09"
	TST_COUNT=9
	RC=0

	# Verify that test_kill_sigstop_t cannot send KILL to the server.
	runcon -t test_kill_sigstop_t -- kill -s KILL $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test10()
{
	TCID="test10"
	TST_COUNT=10
	RC=0

	# Verify that test_kill_sigkill_t cannot send TERM to the server.
	runcon -t test_kill_sigkill_t -- kill -s TERM $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test11()
{
	TCID="test11"
	TST_COUNT=11
	RC=0

	# Verify that test_kill_sigkill_t cannot send CHLD to the server.
	runcon -t test_kill_sigkill_t -- kill -s CHLD $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test12()
{
	TCID="test12"
	TST_COUNT=12
	RC=0

	# Verify that test_kill_sigkill_t cannot send STOP to the server.
	runcon -t test_kill_sigkill_t -- kill -s STOP $PID 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "sigkill passed."
		RC=0
	else
		tst_resm TFAIL "sigkill failed."
		RC=1
	fi
	return $RC
}

test13()
{
	TCID="test13"
	TST_COUNT=13
	RC=0

	# Verify that test_kill_signal_t can send a TERM signal to the server.
	runcon -t test_kill_signal_t -- kill -s TERM $PID 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "sigkill passed."
	else
		tst_resm TFAIL "sigkill failed."
	fi
	return $RC
}

test14()
{
	TCID="test14"
	TST_COUNT=14
	RC=0

	# Verify that test_kill_sigchld_t can send
	# a CHLD signal to the server.
	runcon -t test_kill_sigchld_t -- kill -s CHLD $PID 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "sigkill passed."
	else
		tst_resm TFAIL "sigkill failed."
	fi
	return $RC
}

test15()
{
	TCID="test15"
	TST_COUNT=15
	RC=0

	# Verify that test_kill_sigstop_t can send 
	# a STOP signal to the server.
	runcon -t test_kill_sigstop_t -- kill -s STOP $PID 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "sigkill passed."
	else
		tst_resm TFAIL "sigkill failed."
	fi
	return $RC
}

test16()
{
	TCID="test16"
	TST_COUNT=16
	RC=0

	# Resume the server.
	kill -s CONT $PID

	# Verify that test_kill_sigkill_t can send a KILL signal 
	# to the server.
	runcon -t test_kill_sigkill_t -- kill -s KILL $PID 2>&1
	RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "sigkill passed."
	else
		tst_resm TFAIL "sigkill failed."
	fi
	return $RC
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
test11 || EXIT_VAL=$RC
test12 || EXIT_VAL=$RC
test13 || EXIT_VAL=$RC
test14 || EXIT_VAL=$RC
test15 || EXIT_VAL=$RC
test16 || EXIT_VAL=$RC
exit $EXIT_VAL
