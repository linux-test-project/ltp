#!/bin/sh
#
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Zhang Jin <jy_zhangjin@cn.fujitsu.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
# the GNU General Public License for more details.
#
# Test df command with some basic options.
#

TCID=df01
TST_TOTAL=12
. test.sh

setup()
{
	tst_require_root

	tst_check_cmds df mkfs.${FS_TYPE} stat

	tst_tmpdir

	TST_CLEANUP="cleanup"

	tst_acquire_device

	tst_mkfs ${FS_TYPE} ${TST_DEVICE}

	ROD_SILENT mkdir -p mntpoint

	mount ${TST_DEVICE} mntpoint
	ret=$?
	if [ $ret -eq 32 ]; then
		tst_brkm TCONF "Cannot mount ${FS_TYPE}, missing driver?"
	fi

	if [ $ret -ne 0 ]; then
		tst_brkm TBROK "Failed to mount device: mount exit = $ret"
	fi

	DF_FS_TYPE=$(mount | grep "$TST_DEVICE" | awk '{print $5}')
}

cleanup()
{
	tst_umount ${TST_DEVICE}

	tst_release_device

	tst_rmdir
}

usage()
{
	cat << EOF
	usage: $0 [-f <ext2|ext3|ext4|vfat|...>]

	OPTIONS
		-f	Specify the type of filesystem to be built.  If not
			specified, the default filesystem type (currently ext2)
			is used.
		-h	Display help text and exit.

EOF
	tst_brkm TCONF "Display help text or unknown options"
}

df_test()
{
	df_verify $1
	if [ $? -ne 0 ]; then
		return
	fi

	df_check $1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'$1' failed, not expected."
		return
	fi

	ROD_SILENT dd if=/dev/zero of=mntpoint/testimg bs=1024 count=1024

	df_verify $1

	df_check $1
	if [ $? -eq 0 ]; then
		tst_resm TPASS "'$1' passed."
	else
		tst_resm TFAIL "'$1' failed."
	fi

	ROD_SILENT rm -rf mntpoint/testimg

	# flush file system buffers, then we can get the actual sizes.
	sync
}

df_verify()
{
	$@ >output 2>&1
	if [ $? -ne 0 ]; then
		grep -q -E "unrecognized option | invalid option" output
		if [ $? -eq 0 ]; then
			tst_resm TCONF "'$1' not supported."
			return 32
		else
			tst_resm TFAIL "'$1' failed."
			cat output
			return 1
		fi
	fi
}

df_check()
{
	if [ "$(echo $@)" = "df -i" ]; then
		local total=$(stat -f mntpoint --printf=%c)
		local free=$(stat -f mntpoint --printf=%d)
		local used=$((total-free))
	else
		local total=$(stat -f mntpoint --printf=%b)
		local free=$(stat -f mntpoint --printf=%f)
		local used=$((total-free))
		local bsize=$(stat -f mntpoint --printf=%s)
		total=$(($total * $bsize / 1024))
		used=$(($used * $bsize / 1024))
	fi

	grep ${TST_DEVICE} output | grep -q "${total}.*${used}"
	if [ $? -ne 0 ]; then
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
	df_test "df -t ${DF_FS_TYPE}"
}

test6()
{
	df_test "df -T"
}

test7()
{
	df_test "df -v ${TST_DEVICE}"
}

test8()
{
	df_verify "df -h"
	if [ $? -eq 0 ]; then
		tst_resm TPASS "'df -h' passed."
	fi
}

test9()
{
	df_verify "df -H"
	if [ $? -eq 0 ]; then
		tst_resm TPASS "'df -H' passed."
	fi
}

test10()
{
	df_verify "df -m"
	if [ $? -eq 0 ]; then
		tst_resm TPASS "'df -m' passed."
	fi
}

test11()
{
	df_verify "df --version"
	if [ $? -eq 0 ]; then
		tst_resm TPASS "'df --version' passed."
	fi
}

test12()
{
	df_verify "df -x ${DF_FS_TYPE}"
	if [ $? -ne 0 ]; then
		return
	fi

	grep ${TST_DEVICE} output | grep -q mntpoint
	if [ $? -ne 0 ]; then
		tst_resm TPASS "'df -x ${FS_TYPE}' passed."
	else
		tst_resm TFAIL "'df -x ${FS_TYPE}' failed."
	fi
}

FS_TYPE=ext2
while getopts f:h: OPTION; do
	case $OPTION in
	f)
		FS_TYPE=$OPTARG;;
	h)
		usage;;
	?)
		usage;;
	esac
done

setup

for i in $(seq 1 ${TST_TOTAL})
do
	test$i
done

tst_exit
