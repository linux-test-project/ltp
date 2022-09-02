#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015 Fujitsu Ltd.
# Copyright (c) 2018-2022 Petr Vorel <pvorel@suse.cz>
# Author: Zhang Jin <jy_zhangjin@cn.fujitsu.com>
#
# Test df command with some basic options.

TST_ALL_FILESYSTEMS=1
TST_MOUNT_DEVICE=1
TST_CNT=12
TST_SETUP=setup
TST_TESTFUNC=test
TST_NEEDS_ROOT=1

setup()
{
	DF_FS_TYPE="$(grep -E "$TST_MNTPOINT ($TST_FS_TYPE|fuseblk)" /proc/mounts | awk 'NR==1{print $3}')"
}

df_test()
{
	local cmd="$1 -P"

	df_verify $cmd
	if [ $? -ne 0 ]; then
		return
	fi

	df_check $cmd
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'$cmd' failed, not expected."
		return
	fi

	ROD_SILENT dd if=/dev/zero of=$TST_MNTPOINT/testimg bs=1024 count=1024

	df_verify $cmd

	df_check $cmd
	if [ $? -eq 0 ]; then
		tst_res TPASS "'$cmd' passed."
	else
		tst_res TFAIL "'$cmd' failed."
	fi

	ROD_SILENT rm -rf $TST_MNTPOINT/testimg

	# flush file system buffers, then we can get the actual sizes.
	sync
}

df_verify()
{
	$@ >output 2>&1
	if [ $? -ne 0 ]; then
		grep -q -E "unrecognized option | invalid option" output
		if [ $? -eq 0 ]; then
			tst_res TCONF "'$@' not supported."
			return 32
		else
			tst_res TFAIL "'$@' failed."
			cat output
			return 1
		fi
	fi
}

df_check()
{
	if [ "$(echo $@)" = "df -i -P" ]; then
		local total=$(stat -f $TST_MNTPOINT --printf=%c)
		local free=$(stat -f $TST_MNTPOINT --printf=%d)
		local used=$((total-free))
	else
		local total=$(stat -f $TST_MNTPOINT --printf=%b)
		local free=$(stat -f $TST_MNTPOINT --printf=%f)
		local used=$((total-free))
		local bsize=$(stat -f $TST_MNTPOINT --printf=%s)
		total=$((($total * $bsize + 512)/ 1024))
		used=$((($used * $bsize + 512) / 1024))
	fi

	grep $TST_DEVICE output | grep -q "${total}.*${used}"
	if [ $? -ne 0 ]; then
		echo "total: ${total}, used: ${used}"
		echo "df saved output:"
		cat output
		echo "df output:"
		$@
		return 1
	fi
}

test1()
{
	df_test "df"
}

test2()
{
	df_test "df -a"
}

test3()
{
	df_test "df -i"
}

test4()
{
	df_test "df -k"
}

test5()
{
	df_test "df -t $DF_FS_TYPE"
}

test6()
{
	df_test "df -T"
}

test7()
{
	df_test "df -v $TST_DEVICE"
}

test8()
{
	df_verify "df -h"
	if [ $? -eq 0 ]; then
		tst_res TPASS "'df -h' passed."
	fi
}

test9()
{
	df_verify "df -H"
	if [ $? -eq 0 ]; then
		tst_res TPASS "'df -H' passed."
	fi
}

test10()
{
	df_verify "df -m"
	if [ $? -eq 0 ]; then
		tst_res TPASS "'df -m' passed."
	fi
}

test11()
{
	df_verify "df --version"
	if [ $? -eq 0 ]; then
		tst_res TPASS "'df --version' passed."
	fi
}

test12()
{
	local fs="$DF_FS_TYPE"

	local cmd="df -x $fs -P"

	df_verify $cmd
	if [ $? -ne 0 ]; then
		return
	fi

	grep $TST_DEVICE output | grep -q $TST_MNTPOINT
	if [ $? -ne 0 ]; then
		tst_res TPASS "'$cmd' passed."
	else
		tst_res TFAIL "'$cmd' failed."
	fi
}

. tst_test.sh
tst_run
