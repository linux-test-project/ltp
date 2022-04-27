#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2000
# Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
# Author: Robbie Williamson <robbiew@us.ibm.com>
#
# Test basic functionality of the `ld` command.

CC=${CC:=gcc}
LD=${LD:=ld}

TST_CNT=5
TST_TESTFUNC=test
TST_SETUP=setup
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="$CC $LD"

setup()
{
	for i in rf1 f1 rd1 d1 main; do
		ROD $CC -fPIC -c -o ${i}.o $TST_DATAROOT/${i}.c
	done
}

test1()
{
	EXPECT_FAIL $LD x.o y.o 2\> ld.out

	if grep -q "[xy]\.o.*No such file or directory" ld.out; then
		tst_res TPASS "Missing files were reported"
	else
		tst_res TFAIL "Missing files were not reported"
		cat ld.out
	fi
}

test2()
{
	EXPECT_FAIL $CC x.o y.o 2\> cc.out

	if grep -q "[xy]\.o.*No such file or directory" cc.out; then
		tst_res TPASS "Missing files were reported"
	else
		tst_res TFAIL "Missing files were not reported"
		cat cc.out
	fi
}

test3()
{
	EXPECT_PASS $LD -shared f1.o d1.o -o test.so

	if file test.so |grep -q -e 'pie executable' -e 'shared object'; then
		tst_res TPASS "Shared library could be build"
	else
		tst_res TFAIL "Failed to build shared library"
	fi
}

test4()
{
	EXPECT_PASS $LD -Bdynamic -shared f1.o d1.o -o test.so
	EXPECT_FAIL $LD -Bstatic -L. main.o rd1.o test.so -o a.out
}

test5()
{
	EXPECT_PASS $LD -Bdynamic -shared main.o f1.o rf1.o -o test.so -L/usr/lib/
	EXPECT_FAIL $LD -Bstatic -r main.o f1.o rf1.o test.so -L/usr/lib/ 2\> ld.out
	cat ld.out

	if grep -q "$LD: attempted static link of dynamic object" ld.out; then
		tst_res TPASS "Got expected error message"
	else
		tst_res TFAIL "Unexpected error message"
		cat ld.out
	fi
}

. tst_test.sh
tst_run
