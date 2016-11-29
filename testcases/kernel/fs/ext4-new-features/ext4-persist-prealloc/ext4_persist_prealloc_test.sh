#!/bin/bash

################################################################################
##                                                                            ##
## Copyright (c) 2009 FUJITSU LIMITED                                         ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software Foundation,   ##
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           ##
##                                                                            ##
## Author: Li Zefan <lizf@cn.fujitsu.com>                                     ##
##         Miao Xie <miaox@cn.fujitsu.com>                                    ##
##                                                                            ##
################################################################################

export TCID="ext4-persistent-preallocation"
export TST_TOTAL=2

. ext4_funcs.sh

# Use ltp's syscall/fallocate to test this feature
# $1: 1024 or 4096
ext4_test_persist_prealloc()
{
	mkfs.ext4 -I 256 -b $1 $EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to create ext4 filesystem"
		return
	fi

	mount -t ext4 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount ext4 filesystem"
		return
	fi

	for ((i = 1; i <= 3; i++))
	{
		if ! command -v fallocate0${i} > /dev/null 2>&1; then
			tst_resm TFAIL "file - fallocate0${i} doesn't exist. Please \
				check whether it was compiled and installed.\
				(Path: LTPROOT/testcases/kernel/syscalls/fallocate)"
			umount mnt_point
			return
		fi

		temp_tmpdir=$TMPDIR
		TMPDIR=${PWD}/mnt_point; fallocate0${i} > /dev/null 2>&1
		ret=$?
		TMPDIR=$temp_tmpdir

		if [ $ret -ne 0 ]; then
			tst_resm TFAIL "fallocate's return value is not expected"
			umount mnt_point
			return
		fi
	}

	umount mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to umount ext4 filesystem"
		return
	fi

	e2fsck -p $EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "fsck returned failure"
		return
	fi

	tst_resm TPASS "ext4 persistent preallocation test pass"
}

# main
ext4_setup

ext4_test_persist_prealloc 1024
ext4_test_persist_prealloc 4096

tst_exit
