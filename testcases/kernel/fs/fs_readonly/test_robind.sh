#!/bin/sh
#
#   Copyright (c) International Business Machines  Corp., 2008
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#*******************************************************************************
# Readme_ROBind has more details on the tests running for ROBIND.
# TEST:
#   NAME:       test_robind.sh
#   FUNCTIONALITY:  File system tests for normal mount, bind mount and RO mount
#
#   DESCRIPTION:    Performs filesystems tests for RO mount.
#     For filesystem, like ext2, ext3, reiserfs, jfs & xfs,
#     This test needs a big block device(>=500MB is ok), and you can specify
#     it by -z option when running runltp.
#        a)  mounts on dir1,
#        b)  mount --bind dir2
#        c)  mount -o remount,ro
#       It verifies the tests on a) and b) works correctly.
#       For the c) option it checks that the tests are not able to write
#       into dir.
#===============================================================================
#
# CHANGE HISTORY:
# DATE           AUTHOR                  REASON
# 09/06/2008     Veerendra Chandrappa    For Container, testing of RO-Bind mount
#                Dave Hansen
# This script is based on the Dave Hansen script for testing the robind.
#*******************************************************************************

export TCID="test_robind"
export TST_TOTAL=3

DIRS="dir1 dir2-bound dir3-ro"
dir1_mount_flag=0
dir2_bound_mount_flag=0
dir3_ro_mount_flag=0

. test.sh

usage()
{
	cat << EOF
	usage: $0 -c command [ext3,ext2,jfs,xfs,reiserfs,ramfs]

	This script verifies ReadOnly-filesystem, by mounting block device and
	executing the filesystem tests.

	OPTIONS
		-h    display this message and exit
		-c    command to be executed

EOF
	exit 1
}

umount_mntpoint()
{
	if [ $dir3_ro_mount_flag -eq 1 ];then
		umount dir3-ro
		if [ $? -ne 0 ];then
			tst_resm TWARN "umount dir3-ro failed"
		else
			dir3_ro_mount_flag=0
		fi
	fi

	if [ $dir2_bound_mount_flag -eq 1 ];then
		umount dir2-bound
		if [ $? -ne 0 ];then
			tst_resm TWARN "umount dir2-bound failed"
		else
			dir2_bound_mount_flag=0
		fi
	fi

	if [ $dir1_mount_flag -eq 1 ];then
		umount dir1
		if [ $? -ne 0 ];then
			tst_resm TWARN "umount dir1"
		else
			dir1_mount_flag=0
		fi
	fi
}

cleanup()
{
	umount_mntpoint
	tst_rmdir
}

# parameters: file_systems (if any )
setup()
{
	tst_require_root

	if [ -z "$LTP_BIG_DEV" ];then
		tst_brkm TCONF "tests need a big block device(>=500MB)"
	else
		device=$LTP_BIG_DEV
	fi

	tst_tmpdir
	TST_CLEANUP=cleanup

	for dir in $DIRS
	do
		rm -rf $dir
		mkdir -p $dir
	done

	# populating the default FS as $LTP_BIG_DEV_FS_TYPE
	# (or ext3 if it's not set), if FS is not given
	if [ -z "$*" ]; then
		FSTYPES=${LTP_BIG_DEV_FS_TYPE:-ext3}
	else
		FSTYPES="$*"
	fi
}

# the core function where it runs the tests
# $1 - directory where to run tests
# $2 - file system type
# $3 - read-only flag [true|false]
testdir()
{
	local dir=$1
	local fs_type=$2
	local RO=$3
	local tst_result=0
	local curdir=$(pwd)

	cd $dir
	tst_resm TINFO "command: $command"

	# we need to export TMPDIR, in case test calls tst_rmdir()
	export TMPDIR=$curdir/$dir

	eval $command > $curdir/test.log 2>&1
	tst_result=$?

	# if tst_result isn't 0 and read-only flag is false, the test failed
	# or if tst_result is 0 and read-only flag is true, the test failed.
	if [ "$RO" = "false" -a $tst_result -ne 0 -o "$RO" = "true" -a \
	     $tst_result -eq 0 ];then
		tst_resm TINFO "error info:"
		cat $curdir/test.log
		tst_resm TFAIL "RO-FileSystem Tests FAILED for \
				$dir $fs_type read-only flag: $RO"
	else
		tst_resm TPASS "RO-FileSystem Tests PASSED for \
				$dir $fs_type read-only flag: $RO"
	fi

	# remove all the temp files created.
	cd ..
	rm -f $curdir/test.log
	rm -rf $curdir/$dir/*
}

#=============================================================================
# MAIN
#     See the description, purpose, and design of this test under TEST
#     in this test's prolog.
#=============================================================================

while getopts c:h: OPTION; do
	case $OPTION in
	c)
		command=$OPTARG;;
	h)
		usage;;
	?)
		usage;;
	esac
done
shift $((OPTIND-1))

setup $*

# Executes the tests for different FS's
for fstype in $FSTYPES; do
	if [ "$fstype" = "reiserfs" ]; then
		opts="-f --journal-size 513 -q"
	elif echo "$fstype" | grep -q "ext"; then
		opts="-F"
	elif [ "$fstype" = "xfs" ]; then
		opts="-f"
	elif [ "$fstype" = "btrfs" ]; then
		opts="-f"
	fi

	if [ "$fstype" != "ramfs" ]; then
		tst_mkfs $fstype $device $opts
	fi

	mount -t $fstype $device  dir1
	if [ $? -ne 0 ];then
		tst_brkm TBROK "mount $device to dir1 failed"
	else
		dir1_mount_flag=1
	fi

	mount --bind dir1 dir2-bound
	if [ $? -ne 0 ];then
		tst_brkm TBROK "mount --bind dir1 dir2-bound failed"
	else
		dir2_bound_mount_flag=1
	fi

	mount --bind dir1 dir3-ro
	if [ $? -ne 0 ];then
		tst_brkm TBROK "mount --bind dir1 dir3-ro failed"
	else
		dir3_ro_mount_flag=1
	fi

	mount -o remount,ro,bind dir1 dir3-ro
	if [ $? -ne 0 ];then
		tst_brkm TBROK "mount -o remount,ro,bind dir1 dir3-ro failed"
	fi

	testdir dir1 $fstype false
	testdir dir2-bound $fstype false
	testdir dir3-ro $fstype true
	umount_mntpoint
done

tst_exit
