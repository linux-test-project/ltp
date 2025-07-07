#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
#
# Test du command with some basic options.

TST_CNT=23
TST_SETUP=setup
TST_TESTFUNC=do_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="dd du stat"

setup()
{
	ROD_SILENT mkdir basedir
	cd basedir || tst_brk TBROK "cd basedir failed"

	ROD_SILENT dd if=/dev/zero of=testfile bs=1M count=10

	ROD_SILENT mkdir -p testdir

	ROD_SILENT ln -s ../testfile testdir/testsymlink

	# Display values are in units of the first available SIZE
	# from --block-size, and the DU_BLOCK_SIZE, BLOCK_SIZE and
	# BLOCKSIZE environment variables. Here we need to
	# set DU_BLOCK_SIZE to 1024 to ensure the result is expected.
	export DU_BLOCK_SIZE=1024

	# Some subtests are language dependent
	export LC_ALL=C
}

du_test()
{
	local test_return

	$1 > ../temp 2>&1
	test_return=$?

	if [ ${test_return} -ne 0 ]; then
		grep -q -E "unrecognized option|invalid option" ../temp
		if [ $? -eq 0 ]; then
			tst_res TCONF "'$1' not supported"
		else
			tst_res TFAIL "'$1' failed"
		fi
		return
	fi

	grep -q $2 ../temp
	if [ $? -eq 0 ]; then
		tst_res TPASS "'$1' passed"
	else
		tst_res TFAIL "'$1' failed"
		tst_res TINFO "Looking for '$2' in:"
		cat ../temp
	fi
}

block_size_default=512

block_size=$(stat -f --format="%s" .)
block_size=$((block_size / 1024))

# The output could be different in some systems, if we use du to
# estimate file space usage with the same filesystem and the same size.
# So we use the approximate value to check.
check1="^10[2-3][0-9][0-9][[:space:]]\."
check2="^10[2-3][0-9][0-9][[:space:]]testfile"
check3="^\(0\|${block_size}\)[[:space:]]\.\/testdir\/testsymlink"
check5="^20[4-6][0-9][0-9][[:space:]]\."
check7="^10[4-5][0-9][0-9]\{4\}[[:space:]]\."
check9="^10[2-3][0-9][0-9][[:space:]]total"
check11="^10[2-3][0-9][0-9][[:space:]]testdir\/testsymlink"
check14="^1[0,1]M[[:space:]]\."
check16="^10[2-3][0-9][0-9][[:space:]]testdir\/"
check20="^11M[[:space:]]\."
check23="^[0-9]\{1,2\}[[:space:]]\."

do_test()
{
	case $1 in
	1) du_test "du" ${check1};;
	2) du_test "du testfile" ${check2};;
	3) du_test "du -a" ${check3};;
	4) du_test "du --all" ${check3};;
	5) du_test "du -B ${block_size_default}" ${check5};;
	6) du_test "du --block-size=${block_size_default}" ${check5};;
	7) du_test "du -b" ${check7};;
	8) du_test "du --bytes" ${check7};;
	9) du_test "du -c" ${check9};;
	10) du_test "du --total" ${check9};;
	11) du_test "du -D testdir/testsymlink" ${check11};;
	12) du_test "du --dereference-args testdir/testsymlink" ${check11};;
	13) du_test "du --max-depth=1" ${check1};;
	14) du_test "du --human-readable" ${check14};;
	15) du_test "du -k" ${check1};;
	16) du_test "du -L testdir/" ${check16};;
	17) du_test "du --dereference testdir/" ${check16};;
	18) du_test "du -P" ${check1};;
	19) du_test "du --no-dereference" ${check1};;
	20) du_test "du --si" ${check20};;
	21) du_test "du -s" ${check1};;
	22) du_test "du --summarize" ${check1};;
	23) du_test "du --exclude=testfile" ${check23};;
	esac
}

. tst_test.sh
tst_run
