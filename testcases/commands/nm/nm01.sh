#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2000
# Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
# Author: Robbie Williamson <robbiew@us.ibm.com>
#
# Tests the basic functionality of the `nm` command.

NM=${NM:=nm}

TST_CNT=7
TST_TESTFUNC=test
TST_SETUP=setup
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="$NM"

setup()
{
	ROD cp "$TST_DATAROOT/lib.a" "."
	ROD mkdir "dir"
	ROD cp "$TST_DATAROOT/lib.a" "dir/"
}

test1()
{
	EXPECT_PASS $NM -f posix -A "lib.a" \> nm.out

	if grep -q "lib.a\[f2.o\]\:" nm.out; then
		tst_res TPASS "Got correct listing"
	else
		tst_res TFAIL "Got incorrect listing"
		cat nm.out
	fi

	EXPECT_PASS $NM -f posix -A "dir/lib.a" \> nm.out

	if grep -q "dir/lib.a\[f2.o\]\:" nm.out; then
		tst_res TPASS "Got correct listing"
	else
		tst_res TFAIL "Got incorrect listing"
		cat nm.out
	fi
}

test2()
{
	EXPECT_PASS $NM -f posix -g $TST_DATAROOT/f1 \> nm.out

	if grep -q "^[^ ]\+ [abdft]" nm.out; then
		tst_res TFAIL "Got internal symbols with -g"
		cat nm.out
	else
		tst_res TPASS "Got only external symbols with -g"
	fi
}

test3()
{
	EXPECT_PASS $NM -f posix -t o $TST_DATAROOT/f1 \> nm.out

	if awk '{print $3}' nm.out | grep -q "[8-9a-f]"; then
		tst_res TFAIL "Got non-octal symbol values with -f"
		cat nm.out
	else
		tst_res TPASS "Got an octal symbol values with -f"
	fi
}

test4()
{
	EXPECT_PASS $NM -f sysv $TST_DATAROOT/f1 \> nm.out

	if grep -q "Name" nm.out; then
		tst_res TPASS "Got SysV format with -f sysv"
	else
		tst_res TFAIL "Got wrong format with -f sysv"
		cat nm.out
	fi
}

test5()
{
	EXPECT_PASS $NM -f bsd $TST_DATAROOT/f1 \> nm_bsd.out
	EXPECT_PASS $NM -f posix $TST_DATAROOT/f1 \> nm_posix.out

	ROD awk '{print gensub(/\y(0+)([0-9a-fA-F]+)\y/, "\\2", "g")}' nm_bsd.out \> trimmed_nm_bsd.out
	ROD awk '{print gensub(/\y(0+)([0-9a-fA-F]+)\y/, "\\2", "g")}' nm_posix.out \> trimmed_nm_posix.out

	ROD awk '{print $3 $2 $1}' trimmed_nm_bsd.out \> nm1.out
	ROD awk '{print $1 $2 $3}' trimmed_nm_posix.out \> nm2.out

	if diff nm1.out nm2.out > /dev/null; then
		tst_res TPASS "Got BSD format with -f bsd"
	else
		tst_res TFAIL "Got wrong format with -f bsd"
		cat nm_bsd.out
	fi
}

test6()
{
	EXPECT_PASS $NM -f sysv -u $TST_DATAROOT/f1 \> nm.out

	if grep -q "Undefined symbols from" nm.out; then
		tst_res TPASS "Got undefined symbols with -u"
	else
		tst_res TFAIL "Haven't got undefined symbols with -u"
		cat nm.out
	fi
}

test7()
{
	EXPECT_PASS $NM -s $TST_DATAROOT/lib.a \> nm.out

	if grep -q "index" nm.out; then
		tst_res TPASS "Got index with -s"
	else
		tst_res TFAIL "Haven't got index with -s"
	fi
}

. tst_test.sh
tst_run
