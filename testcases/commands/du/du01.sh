#!/bin/sh
#
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Test du command with some basic options.
#

TCID=du01
TST_TOTAL=23
. test.sh

setup()
{
	tst_check_cmds dd du stat

	tst_tmpdir
	TST_CLEANUP=cleanup

	ROD_SILENT dd if=/dev/zero of=testfile bs=1M count=10

	ROD_SILENT mkdir -p testdir

	ROD_SILENT ln -s ../testfile testdir/testsymlink

	# Display values are in units of the first available SIZE
	# from --block-size, and the DU_BLOCK_SIZE, BLOCK_SIZE and
	# BLOCKSIZE environment variables. Here we need to
	# set DU_BLOCK_SIZE to 1024 to ensure the result is expected.
	export DU_BLOCK_SIZE=1024
}

cleanup()
{
	tst_rmdir
}

du_test()
{
	local test_return

	$1 > temp 2>&1
	test_return=$?

	if [ ${test_return} -ne 0 ]; then
		grep -q -E "unrecognized option|invalid option" temp
		if [ $? -eq 0 ]; then
			tst_resm TCONF "'$1' not supported"
		else
			tst_resm TFAIL "'$1' failed"
		fi
		return
	fi

	grep -q $2 temp
	if [ $? -eq 0 ]; then
		tst_resm TPASS "'$1' passed"
	else
		tst_resm TFAIL "'$1' failed"
		tst_resm TINFO "Looking for '$2' in:"
		cat temp
	fi
}

setup

block_size=512

# The output could be different in some systems, if we use du to
# estimate file space usage with the same filesystem and the same size.
# So we use the approximate value to check.
check1="10[2-3][0-9][0-9][[:space:]]\."
check2="10[2-3][0-9][0-9][[:space:]]testfile"
check3="[0-4][[:space:]]\.\/testdir\/testsymlink"
check5="20[4-5][0-9][0-9][[:space:]]\."
check7="10[4-5][0-9][0-9]\{4\}[[:space:]]\."
check9="10[2-3][0-9][0-9][[:space:]]total"
check11="10[2-3][0-9][0-9][[:space:]]testdir\/testsymlink"
check14="1[0,1]M[[:space:]]\."
check16="10[2-3][0-9][0-9][[:space:]]testdir\/"
check20="11M[[:space:]]\."
check23="[0-9]\{1,2\}[[:space:]]\."

du_test "du" ${check1}
du_test "du testfile" ${check2}
du_test "du -a" ${check3}
du_test "du --all" ${check3}
du_test "du -B ${block_size}" ${check5}
du_test "du --block-size=${block_size}" ${check5}
du_test "du -b" ${check7}
du_test "du --bytes" ${check7}
du_test "du -c" ${check9}
du_test "du --total" ${check9}
du_test "du -D testdir/testsymlink" ${check11}
du_test "du --dereference-args testdir/testsymlink" ${check11}
du_test "du --max-depth=1" ${check1}
du_test "du --human-readable" ${check14}
du_test "du -k" ${check1}
du_test "du -L testdir/" ${check16}
du_test "du --dereference testdir/" ${check16}
du_test "du -P" ${check1}
du_test "du --no-dereference" ${check1}
du_test "du --si" ${check20}
du_test "du -s" ${check1}
du_test "du --summarize" ${check1}
du_test "du --exclude=testfile" ${check23}

tst_exit
