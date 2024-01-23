#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2001
# Author: Manoj Iyer <manjo@mail.utexas.edu>
#
# Test basic functionality of mv command
# Test #1:  mv <dir1> <dir2>
# move dir1 to dir2 and all its contents.
# Test #2:  mv -b <file1> <file2>
# move file1 to file2 and backup the file2.

TST_CNT=2
TST_SETUP=setup
TST_TESTFUNC=test
TST_NEEDS_TMPDIR=1

setup()
{
	ROD_SILENT mkdir -p tst_mv.old
}

creat_dirnfiles()
{
	local numdirs=$2
	local numfiles=$3
	local dirname=$4
	local dircnt=0
	local fcnt=0

	tst_res TINFO "Test #$1: Creating $numdirs directories."
	tst_res TINFO "Test #$1: filling each dir with $numfiles files."
	while [ $dircnt -lt $numdirs ]
	do
		dirname=$dirname/d.$dircnt
		ROD_SILENT mkdir -p $dirname

		fcnt=0
		while [ $fcnt -lt $numfiles ]
		do
			ROD_SILENT touch $dirname/f.$fcnt
			fcnt=$(($fcnt+1))
		done
		dircnt=$(($dircnt+1))
	done
}

creat_expout()
{
	local numdir=$1
	local numfile=$2
	local dirname=$3
	local dircnt=0
	local fcnt=0

	echo "$dirname:"  1>>tst_mv.exp
	echo "d.$dircnt"  1>>tst_mv.exp
	while [ $dircnt -lt $numdirs ]
	do
		dirname=$dirname/d.$dircnt
		dircnt=$(($dircnt+1))
		echo "$dirname:"  1>>tst_mv.exp
		if [ $dircnt -lt $numdirs ]; then
			echo "d.$dircnt" 1>>tst_mv.exp
		fi

		fcnt=0
		while [ $fcnt -lt $numfiles ]
		do
			echo "f.$fcnt " 1>>tst_mv.exp
			fcnt=$(($fcnt+1))
		done
		printf "\n\n" 1>>tst_mv.exp
	done
}

test1()
{
	numdirs=10
	numfiles=10
	dircnt=0
	fcnt=0

	tst_res TINFO "Test #1: mv <dir1> <dir2> will move dir1 to dir2 and" \
		       "all its contents"

	creat_dirnfiles 1 $numdirs $numfiles tst_mv.old

	mv tst_mv.old tst_mv.new > tst_mv.err 2>&1
	if [ $? -ne 0 ]; then
		cat tst_mv.err
		tst_res TFAIL "Test #1: 'mv tst_mv.old tst_mv.new' failed"
		return
	fi

	tst_res TINFO "Test #1: creating output file"
	ls -R tst_mv.new > tst_mv.out 2>&1

	tst_res TINFO "Test #1: creating expected output file"
	creat_expout $numdirs $numfiles tst_mv.new

	tst_res TINFO "Test #1: comparing expected out and actual output file"
	diff -w -B -q tst_mv.out tst_mv.exp > tst_mv.err 2>&1
	if [ $? -ne 0 ]; then
		cat tst_mv.err
		tst_res TFAIL "Test #1: mv failed."
	else
		tst_res TINFO "Test #1: expected same as actual"
		if [ -f tst_mv.old ]; then
			tst_res TFAIL "Test #1: mv did not delete old" \
				       "directory"
		else
			tst_res TPASS "Test #1: mv success"
		fi
	fi
}

test2()
{
	tst_res TINFO "Test #2: mv -b <file1> <file2> will move dir1 to dir2"

	ROD_SILENT touch tmpfile1 tmpfile2

	MD5_old=$(md5sum tmpfile2 | awk '{print $1}')
	if [ $? -ne 0 ]; then
		tst_brk TBROK "Test #2: can't get the MD5 message of file2."
	fi

	if [ -f "tmpfile2~" ]; then
		tst_brk TBROK "Test #2: file tmpfile2~ should not exists."
	fi

	mv -b tmpfile1 tmpfile2
	if [ $? -ne 0 ]; then
		tst_brk TBROK "Test #2: 'mv -b tmpfile1 tmpfile2' failed."
	fi

	# if 'mv -b file1 file2' succeed, there will be "tmpfile2~" file.

	MD5_backup=$(md5sum tmpfile2 | awk '{print $1}')
	if [ $? -ne 0 ]; then
		tst_brk TBROK "Test #2: can not get the MD5 message of" \
			       "backup file2."
	fi

	if [ "$MD5_old" = "$MD5_backup" -a -f "tmpfile2~" ]; then
		tst_res TPASS "Test #2: mv -b success"
	else
		tst_res TFAIL "Test #2: mv -b failed"
	fi
}

. tst_test.sh
tst_run
