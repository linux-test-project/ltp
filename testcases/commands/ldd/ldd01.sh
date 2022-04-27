#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2000
# Author: Robbie Williamson <robbiew@us.ibm.com>
#
# test basic functionality of the `ldd` command.

TST_CNT=2
TST_SETUP=setup
TST_TESTFUNC=test

LDD=${LDD:=ldd}

setup()
{
	export LD_LIBRARY_PATH="$TST_DATAROOT:$LD_LIBRARY_PATH"
	LDDTESTFILE="$TST_DATAROOT/lddfile.out"
}

test1()
{

	$LDD $LDDTESTFILE | grep -q -E "lddfile1.obj.so|lddfile2.obj.so|lddfile3.obj.so|lddfile4.obj.so|lddfile5.obj.so"
	if [ $? -eq 0 ]; then
		tst_res TPASS "Found lddfile*.obj.so"
	else
		tst_res TFAIL "Haven't found lddfile*.obj.so"
	fi
}

test2()
{
	$LDD -v $LDDTESTFILE | grep -q -E "GLIBC|lddfile1.obj.so|lddfile2.obj.so|lddfile3.obj.so|lddfile4.obj.so|lddfile5.obj.so"
	if [ $? -eq 0 ]; then
		tst_res TPASS "Found GLIBC"
	else
		tst_res TFAIL "Haven't found GLIBC"
	fi
}

. tst_test.sh
tst_run
