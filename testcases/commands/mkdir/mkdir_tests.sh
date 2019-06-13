#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2001
# Author: Manoj Iyer <manjo@mail.utexas.edu>
#
# Basic mkdir tests

TST_CNT=3
TST_SETUP=setup
TST_TESTFUNC=test
TST_NEEDS_TMPDIR=1
. tst_test.sh

setup()
{
	ROD mkdir "dir"
	LONG_PATH="some/long/path/of/several/directories"
}

test1()
{
	EXPECT_FAIL mkdir "dir" 2\> mkdir.out

	if grep -q "dir.*File exists" mkdir.out; then
		tst_res TPASS "Got correct error message"
	else
		tst_res TFAIL "Got wrong error message"
		cat mkdir.out
	fi
}

test2()
{
	EXPECT_FAIL mkdir "$LONG_PATH" 2\> mkdir.out

	if grep -q "$LONG_PATH.*No such file or directory" mkdir.out; then
		tst_res TPASS "Got correct error message"
	else
		tst_res TFAIL "Got wrong error message"
		cat mkdir.out
	fi

	ROD rm -rf "$LONG_PATH"
}

test3()
{
	EXPECT_PASS mkdir -p "$LONG_PATH"

	ROD rm -rf "$LONG_PATH"
}

tst_run
