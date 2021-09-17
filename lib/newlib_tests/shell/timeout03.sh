#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>

# testing shell timeout handling in tst_timeout_kill
# expected output:
# timeout03 1 TINFO: timeout per run is 0h 0m 1s
# timeout03 1 TINFO: testing killing test after TST_TIMEOUT
# Test timed out, sending SIGTERM!
# If you are running on slow machine, try exporting LTP_TIMEOUT_MUL > 1
# Terminated
# timeout03 1 TBROK: test terminated
# timeout03 1 TPASS: test run cleanup after timeout
# Test is still running... 10
# Test is still running... 9
# Test is still running... 8
# Test is still running... 7
# Test is still running... 6
# Test is still running... 5
# Test is still running... 4
# Test is still running... 3
# Test is still running... 2
# Test is still running... 1
# Test is still running, sending SIGKILL
# Killed

TST_TESTFUNC=do_test
TST_CLEANUP=cleanup

TST_TIMEOUT=1
. tst_test.sh

do_test()
{
	tst_res TINFO "testing killing test after TST_TIMEOUT"

	sleep 2
	tst_res TFAIL "test: running after TST_TIMEOUT"
}

cleanup()
{
	tst_res TPASS "test run cleanup after timeout"

	sleep 15 # must be higher than wait time in _tst_kill_test
	tst_res TFAIL "cleanup: running after TST_TIMEOUT"
}

tst_run
