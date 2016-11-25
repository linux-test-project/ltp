#!/bin/sh
#
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
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
# Test mkfs command with some basic options.
#

TST_ID="mkfs01"
TST_CNT=5
TST_SETUP=setup
TST_TESTFUNC=test
TST_OPTS="f:"
TST_USAGE=usage
TST_PARSE_ARGS=parse_args
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_NEEDS_DEVICE=1
TST_NEEDS_CMDS="blkid df"
. tst_test.sh

usage()
{
	cat << EOF
usage: $0 [-f <ext2|ext3|ext4|vfat|...>]

OPTIONS
-f	Specify the type of filesystem to be built.  If not
	specified, the default filesystem type (currently ext2)
	is used.
EOF
}

parse_args()
{
	FS_TYPE="$2"
}

setup()
{
	if [ -n "$FS_TYPE" ]; then
		tst_check_cmds mkfs.${FS_TYPE}
	fi

	ROD_SILENT mkdir -p mntpoint
}

mkfs_mount()
{
	mount ${TST_DEVICE} mntpoint
	local ret=$?
	if [ $ret -eq 32 ]; then
		tst_brk TCONF "Cannot mount ${FS_TYPE}, missing driver?"
	fi

	if [ $ret -ne 0 ]; then
		tst_brk TBROK "Failed to mount device: mount exit = $ret"
	fi
}

mkfs_verify_type()
{
	if [ -z "$1" ]; then
		blkid $2 -t TYPE="ext2" >/dev/null
	else
		if [ "$1" = "msdos" ]; then
			blkid $2 -t TYPE="vfat" >/dev/null
		else
			blkid $2 -t TYPE="$1" >/dev/null
		fi
	fi
}

mkfs_verify_size()
{
	mkfs_mount
	local blocknum=`df -P -B 1k mntpoint | tail -n1 | awk '{print $2}'`
	tst_umount "$TST_DEVICE"

	if [ $blocknum -gt "$2" ]; then
		return 1
	fi

	# Size argument in mkfs.ntfs denotes number-of-sectors which is 512bytes,
	# 1k-block size should be devided by this argument for ntfs verification.
	if [ "$1" = "ntfs" ]; then
		local rate=1024/512
		if [ $blocknum -lt "$(($2/rate*9/10))" ]; then
			return 1
		fi
	else
		if [ $blocknum -lt "$(($2*9/10))" ]; then
			return 1
		fi
	fi

	return 0
}

mkfs_test()
{
	local mkfs_op=$1
	local fs_type=$2
	local fs_op=$3
	local device=$4
	local size=$5

	if [ -n "$fs_type" ]; then
		mkfs_op="-t $fs_type"
	fi

	if [ "$fs_type" = "xfs" ] || [ "$fs_type" = "btrfs" ]; then
		fs_op="$fs_op -f"
	fi

	local mkfs_cmd="mkfs $mkfs_op $fs_op $device $size"

	echo ${fs_op} | grep -q "\-c"
	if [ $? -eq 0 ] && [ "$fs_type" = "ntfs" ]; then
		tst_res TCONF "'${mkfs_cmd}' not supported."
		return
	fi

	if [ -n "$size" ]; then
		if [ "$fs_type" = "xfs" ] || [ "$fs_type" = "btrfs" ]; then
			tst_res TCONF "'${mkfs_cmd}' not supported."
			return
		fi
	fi

	${mkfs_cmd} >temp 2>&1
	if [ $? -ne 0 ]; then
		grep -q -E "unknown option | invalid option" temp
		if [ $? -eq 0 ]; then
			tst_res TCONF "'${mkfs_cmd}' not supported."
			return
		else
			tst_res TFAIL "'${mkfs_cmd}' failed."
			cat temp
			return
		fi
	fi

	if [ -n "$device" ]; then
		mkfs_verify_type "$fs_type" "$device"
		if [ $? -ne 0 ]; then
			tst_res TFAIL "'${mkfs_cmd}' failed, not expected."
			return
		fi
	fi

	if [ -n "$size" ]; then
		mkfs_verify_size "$fs_type" "$size"
		if [ $? -ne 0 ]; then
			tst_res TFAIL "'${mkfs_cmd}' failed, not expected."
			return
		fi
	fi

	tst_res TPASS "'${mkfs_cmd}' passed."
}

test1()
{
	mkfs_test "" "$FS_TYPE" "" "$TST_DEVICE"
}

test2()
{
	mkfs_test "" "$FS_TYPE" "" "$TST_DEVICE" "16000"
}

test3()
{
	mkfs_test "" "$FS_TYPE" "-c" "$TST_DEVICE"
}

test4()
{
	mkfs_test "-V"
}

test5()
{
	mkfs_test "-h"
}

tst_run
