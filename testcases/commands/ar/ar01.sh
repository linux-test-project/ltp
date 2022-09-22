#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2000
# Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>
# Author: Robbie Williamson <robbiew@us.ibm.com>
#
# This is a basic $AR command test.

AR="${AR:=ar}"
TST_CNT=17
TST_SETUP=setup
TST_TESTFUNC=test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="$AR"

setup()
{
	MOD=
	$AR --help | grep "use zero for timestamps and uids/gids (default)" >/dev/null
	[ $? -eq 0 ] && MOD="U"
}

test1()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file3.in
	ROD $AR -ra"$MOD" file1.in lib.a $TST_DATAROOT/file2.in
	ROD $AR -t lib.a \> ar.out

	printf "file1.in\nfile2.in\nfile3.in\n" > ar.exp

	if diff ar.out ar.exp >/dev/null; then
		tst_res TPASS "$AR added new file after another (-a)"
	else
		tst_res TFAIL "$AR failed to add new file after another (-a)"
		cat ar.out
	fi

	ROD rm lib.a
}

test2()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file2.in \
			       $TST_DATAROOT/file3.in $TST_DATAROOT/file4.in
	ROD $AR -ma"$MOD" file1.in lib.a file4.in
	ROD $AR -t lib.a \> ar.out

	printf "file1.in\nfile4.in\nfile2.in\nfile3.in\n" > ar.exp

	if diff ar.out ar.exp > /dev/null; then
		tst_res TPASS "$AR moved file correctly (-ma)"
	else
		tst_res TFAIL "$AR failed to move file (-ma)"
		cat ar.out
	fi

	ROD rm lib.a
}

test3()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file3.in
	ROD $AR -rb"$MOD" file3.in lib.a $TST_DATAROOT/file2.in
	ROD $AR -t lib.a \> ar.out

	printf "file1.in\nfile2.in\nfile3.in\n" > ar.exp

	if diff ar.out ar.exp; then
		tst_res TPASS "$AR added new file before another (-b)"
	else
		tst_res TFAIL "$AR failed to add new file before another (-b)"
		cat ar.out
	fi

	ROD rm lib.a
}

test4()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file3.in \
			       $TST_DATAROOT/file2.in
	ROD $AR -mb"$MOD" file3.in lib.a file2.in
	ROD $AR -t lib.a \> ar.out

	printf "file1.in\nfile2.in\nfile3.in\n" > ar.exp

	if diff ar.out ar.exp > /dev/null; then
		tst_res TPASS "$AR moved file correctly (-mb)"
	else
		tst_res TFAIL "$AR failed to move file (-mb)"
		cat ar.out
	fi

	ROD rm lib.a
}

test5()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in \> ar.out

	if [ -s ar.out ]; then
		tst_res TFAIL "$AR produced output unexpectedly (-c)"
		cat ar.out
	else
		tst_res TPASS "$AR haven't produced output (-c)"
	fi

	ROD rm lib.a
}

test6()
{
	ROD $AR -qc"$MOD" lib.a $TST_DATAROOT/file1.in \> ar.out

	if [ -s ar.out ]; then
		tst_res TFAIL "$AR produced output unexpectedly (-qc)"
		cat ar.out
	else
		tst_res TPASS "$AR haven't produced output (-qc)"
	fi

	ROD rm lib.a
}

test7()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file2.in \
			       $TST_DATAROOT/file3.in
	ROD $AR -d"$MOD" lib.a file1.in file2.in
	ROD $AR -t lib.a \> ar.out

	printf "file3.in\n" > ar.exp

	if diff ar.out ar.exp > /dev/null; then
		tst_res TPASS "$AR deleted files correctly (-d)"
	else
		tst_res TFAIL "$AR messed up when deleting files (-d)"
		cat ar.out
	fi

	ROD rm lib.a
}

test8()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file2.in \
			       $TST_DATAROOT/file3.in
	ROD $AR -d"$MOD" lib.a
	ROD $AR -t lib.a \> ar.out

	printf "file1.in\nfile2.in\nfile3.in\n" > ar.exp

	if diff ar.out ar.exp > /dev/null; then
		tst_res TPASS "$AR deleted nothing (-d with empty list)"
	else
		tst_res TFAIL "$AR deleted files (-d with empty list)"
		cat ar.out
	fi

	ROD rm lib.a
}

test9()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file3.in
	ROD $AR -ri"$MOD" file3.in lib.a $TST_DATAROOT/file2.in
	ROD $AR -t lib.a \> ar.out

	printf "file1.in\nfile2.in\nfile3.in\n" > ar.exp

	if diff ar.out ar.exp >/dev/null; then
		tst_res TPASS "$AR added new file before another (-i)"
	else
		tst_res TFAIL "$AR failed to add new file before another (-i"
		cat ar.out
	fi

	ROD rm lib.a
}

test10()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file3.in \
			       $TST_DATAROOT/file2.in
	ROD $AR -mi"$MOD" file3.in lib.a file2.in
	ROD $AR -t lib.a \> ar.out

	printf "file1.in\nfile2.in\nfile3.in\n" > ar.exp

	if diff ar.out ar.exp > /dev/null; then
		tst_res TPASS "$AR moved file correctly (-mi)"
	else
		tst_res TFAIL "$AR failed to move file (-mi)"
		cat ar.out
	fi

	ROD rm lib.a
}

test11()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file3.in \
			       $TST_DATAROOT/file2.in
	ROD $AR -m"$MOD" lib.a file3.in
	ROD $AR -t lib.a \> ar.out

	printf "file1.in\nfile2.in\nfile3.in\n" > ar.exp

	if diff ar.out ar.exp > /dev/null; then
		tst_res TPASS "$AR moved file correctly (-m)"
	else
		tst_res TFAIL "$AR failed to move file (-m)"
		cat ar.out
	fi

	ROD rm lib.a
}

test12()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file2.in \
			       $TST_DATAROOT/file3.in
	ROD $AR -p"$MOD" lib.a \> ar.out

	printf "This is file one\nThis is file two\nThis is file three\n" > ar.exp

	if diff ar.out ar.exp > /dev/null; then
		tst_res TPASS "$AR printed file content correctly (-p)"
	else
		tst_res TFAIL "$AR failed to print file content (-p)"
		cat ar.out
	fi

	ROD rm lib.a
}

test13()
{

	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file2.in \
			       $TST_DATAROOT/file3.in
	ROD $AR -q"$MOD" lib.a $TST_DATAROOT/file4.in
	ROD $AR -t lib.a \> ar.out

	printf "file1.in\nfile2.in\nfile3.in\nfile4.in\n" > ar.exp

	if diff ar.out ar.exp > /dev/null; then
		tst_res TPASS "$AR appended file correctly (-q)"
	else
		tst_res TFAIL "$AR failed to append file (-q)"
		cat ar.out
	fi

	ROD rm lib.a
}

test14()
{
	ROD touch file0.in
	ROD $AR -cr"$MOD" lib.a file0.in $TST_DATAROOT/file1.in

	file0_mtime1=$($AR -tv lib.a | grep file0.in)
	file1_mtime1=$($AR -tv lib.a | grep file1.in)

	touch -c -t $(date --date='next day' +"%Y%m%d%H%M") file0.in

	ROD $AR -ru"$MOD" lib.a file0.in $TST_DATAROOT/file1.in

	file0_mtime2=$($AR -tv lib.a | grep file0.in)
	file1_mtime2=$($AR -tv lib.a | grep file1.in)

	if [ "$file0_mtime1" = "$file0_mtime2" ]; then
		tst_res TFAIL "$AR haven't updated modified file0 (-u)"
	else
		tst_res TPASS "$AR updated modified file0 (-u)"
	fi

	if [ "$file1_mtime1" = "$file1_mtime2" ]; then
		tst_res TPASS "$AR haven't updated unmodified file1 (-u)"
	else
		tst_res TFAIL "$AR updated unmodified file1 (-u)"
	fi

	ROD rm lib.a file0.in
}

test15()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in
	ROD $AR -tv lib.a \> ar.out

	if grep -q '[rwx-]\{9\} [0-9].*/[0-9].*\s*[0-9].*.*file1.in' ar.out; then
		tst_res TPASS "$AR verbose listing works (-tv)"
	else
		tst_res TFAIL "$AR verbose listing failed (-tv)"
		cat ar.out
	fi

	ROD rm lib.a
}

test16()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file2.in \
			       $TST_DATAROOT/file3.in
	ROD $AR -xv"$MOD" lib.a \> ar.out

	printf "x - file1.in\nx - file2.in\nx - file3.in\n" > ar.exp

	if diff ar.out ar.exp > /dev/null; then
		tst_res TPASS "$AR printed extracted filenames (-xv)"
	else
		tst_res TFAIL "$AR failed to print extracted filenames (-xv)"
		cat ar.out
	fi

	if [ -e file1.in -a -e file2.in -a -e file3.in ]; then
		tst_res TPASS "$AR extracted files correctly"
	else
		tst_res TFAIL "$AR failed to extract files"
	fi

	ROD rm -f lib.a file1.in file2.in file3.in
}

test17()
{
	ROD $AR -cr"$MOD" lib.a $TST_DATAROOT/file1.in $TST_DATAROOT/file2.in
	ROD $AR -xv"$MOD" lib.a file2.in \> ar.out

	printf "x - file2.in\n" > ar.exp

	if diff ar.out ar.exp > /dev/null; then
		tst_res TPASS "$AR printed extracted filename (-xv)"
	else
		tst_res TFAIL "$AR failed to print extracted filename (-xv)"
		cat ar.out
	fi

	if [ -e file2.in ]; then
		tst_res TPASS "$AR extracted file correctly"
	else
		tst_res TFAIL "$AR failed to extract file"
	fi

	ROD rm -f lib.a file2.in
}

. tst_test.sh
tst_run
