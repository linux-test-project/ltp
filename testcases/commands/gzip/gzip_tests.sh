#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2001
# Author: Manoj Iyer <manjo@mail.utexas.edu>
#
# Test basic functionality of gzip and gunzip command
# Test #1: Test that gzip -r will travel directories and
#  compress all the files available.
#
# Test #2: Test that gunzip -r will travel directories and
# uncompress all the files available.

TST_CNT=2
TST_TESTFUNC=test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="gzip gunzip"

creat_dirnfiles()
{
	local numdirs=$2
	local numfiles=$3
	local dirname=$4
	local dircnt=0
	local fcnt=0

	tst_res TINFO "Test #$1: Creating $numdirs directories"
	tst_res TINFO "Test #$1: filling each dir with $numfiles files"
	while [ $dircnt -lt $numdirs ]; do
		dirname=$dirname/d.$dircnt
		ROD_SILENT mkdir -p $dirname

		fcnt=0
		while [ $fcnt -lt $numfiles ]; do
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
	local ext=$4
	local dircnt=0
	local fcnt=0

	echo "$dirname:"  1> tst_gzip.exp
	echo "d.$dircnt"  1>> tst_gzip.exp
	while [ $dircnt -lt $numdirs ]; do
		dirname=$dirname/d.$dircnt
		dircnt=$(($dircnt+1))
		echo "$dirname:"  1>> tst_gzip.exp
		if [ $dircnt -lt $numdirs ]; then
			echo "d.$dircnt"  1>> tst_gzip.exp
		fi
		fcnt=0
		while [ $fcnt -lt $numfiles ]; do
			echo "f.$fcnt$ext " 1>> tst_gzip.exp
			fcnt=$(($fcnt+1))
		done
		printf "\n\n" 1>> tst_gzip.exp
	done
}

test1()
{
	numdirs=10
	numfiles=10
	dircnt=0
	fcnt=0

	ROD_SILENT mkdir tst_gzip.tmp

	tst_res TINFO "Test #1: gzip -r will recursively compress contents" \
			"of directory"

	creat_dirnfiles 1 $numdirs $numfiles tst_gzip.tmp

	gzip -r tst_gzip.tmp > tst_gzip.err 2>&1
	if [ $? -ne 0 ]; then
		cat tst_gzip.err
		tst_res TFAIL "Test #1: gzip -r failed"
		return
	fi

	tst_res TINFO "Test #1: creating output file"
	ls -R tst_gzip.tmp > tst_gzip.out 2>&1

	tst_res TINFO "Test #1: creating expected output file"
	creat_expout $numdirs $numfiles tst_gzip.tmp .gz

	diff -w -B tst_gzip.out tst_gzip.exp > tst_gzip.err 2>&1
	if [ $? -ne 0 ]; then
		cat tst_gzip.err
		tst_res TFAIL "Test #1: gzip failed"
	else
		tst_res TPASS "Test #1: gzip -r success"
	fi

	ROD_SILENT rm -rf tst_gzip.tmp/
}

test2()
{
	numdirs=10
	numfiles=10
	dircnt=0
	fcnt=0

	ROD_SILENT mkdir tst_gzip.tmp

	tst_res TINFO "Test #2: gunzip -r will recursively uncompress" \
			"contents of directory"

	creat_dirnfiles 2 $numdirs $numfiles tst_gzip.tmp

	gzip -r tst_gzip.tmp > tst_gzip.err 2>&1
	if [ $? -ne 0 ]; then
		cat tst_gzip.err
		tst_brk TBROK "Test #2: compressing directory tst_gzip.tmp" \
				"failed"
	fi

	gunzip -r tst_gzip.tmp > tst_gzip.err 2>&1
	if [ $? -ne 0 ]; then
		cat tst_gzip.err
		tst_brk TBROK "Test #2: uncompressing directory" \
				" tst_gzip.tmp failed"
	fi

	tst_res TINFO "Test #2: creating output file"
	ls -R tst_gzip.tmp > tst_gzip.out 2>&1

	tst_res TINFO "Test #2: creating expected output file"
	creat_expout $numdirs $numfiles tst_gzip.tmp

	tst_res TINFO "Test #2: comparing expected out and actual output file"
	diff -w -B tst_gzip.out tst_gzip.exp > tst_gzip.err 2>&1
	if [ $? -ne 0 ]; then
		cat tst_gzip.err
		tst_res TFAIL "Test #2: gunzip failed"
	else
		tst_res TPASS "Test #2: gunzip -r success"
	fi

	ROD_SILENT rm -rf tst_gzip.tmp/
}

. tst_test.sh
tst_run
