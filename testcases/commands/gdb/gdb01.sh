#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2017 Red Hat, Inc.
#
# Test if gdb can successfully attach to a process and
# this process exits normally.

TST_TESTFUNC=simple_test
TST_NEEDS_CMDS="gdb /bin/cat"

. tst_test.sh

simple_test()
{
	gdb /bin/cat -ex "run /etc/passwd" -ex quit < /dev/null
	RC=$?
	if [ $RC -eq 0 ] ; then
		tst_res TPASS "gdb attached to process and completed run"
	else
		tst_res TFAIL "gdb test failed with" $RC
	fi
}

tst_run
