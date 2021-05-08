#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>

# testing shell timeout handling in _tst_kill_test()
# expected output:
# timeout03 1 TINFO: timeout per run is 0h 0m 1s
# timeout03 1 TINFO: testing killing test after TST_TIMEOUT
# timeout03 1 TBROK: Test timeouted, sending SIGINT! If you are running on slow machine, try exporting LTP_TIMEOUT_MUL > 1
# timeout03 1 TBROK: test interrupted or timed out
# timeout03 1 TPASS: test run cleanup after timeout
# timeout03 1 TINFO: Test is still running, waiting 10s
# timeout03 1 TINFO: Test is still running, waiting 9s
# timeout03 1 TINFO: Test is still running, waiting 8s
# timeout03 1 TINFO: Test is still running, waiting 7s
# timeout03 1 TINFO: Test is still running, waiting 6s
# timeout03 1 TINFO: Test is still running, waiting 5s
# timeout03 1 TINFO: Test is still running, waiting 4s
# timeout03 1 TINFO: Test is still running, waiting 3s
# timeout03 1 TINFO: Test is still running, waiting 2s
# timeout03 1 TINFO: Test is still running, waiting 1s
# timeout03 1 TBROK: Test still running, sending SIGKILL
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
