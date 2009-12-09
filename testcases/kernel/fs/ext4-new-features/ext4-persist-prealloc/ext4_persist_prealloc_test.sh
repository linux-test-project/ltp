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
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
## Author: Li Zefan <lizf@cn.fujitsu.com>                                     ##
##         Miao Xie <miaox@cn.fujitsu.com>                                    ##
##                                                                            ##
################################################################################

cd $LTPROOT/testcases/bin

. ./ext4_funcs.sh

export TCID="ext4-persistent-preallocation"
export TST_TOTAL=2
export TST_COUNT=1

export TEST_DIR=$PWD

# $1: the test config
read_config $1

# The test path of fallocate
export TDIRECTORY=$PWD/mnt_point/

# Use ltp's syscall/fallocate to test this feature
# $1: 1024 or 4096
ext4_test_persist_prealloc()
{
	mkfs.ext4 -I 256 -b $1 $EXT4_DEV &> /dev/null
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to create ext4 filesystem"
		return 1
	fi

	mount -t ext4 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount ext4 filesystem"
		return 1
	fi

	ret=1

	for ((i = 1; i <= 3; i++))
	{
		if [ ! -f fallocate0${i} ];  then
			tst_resm TFAIL "file - fallocate0${i} doesn't exist. Please \
				check whether it was compiled and installed.\
				(Path: LTPROOT/testcases/kernel/syscalls/fallocate)"
			umount mnt_point
			return 1
		fi

		./fallocate0${i} | grep -q "CONF"
		if [ $? -ne $ret ]; then
			tst_resm TFAIL "fallocate's return value is not expected"
			umount mnt_point
			return 1
		fi
	}

	umount mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to umount ext4 filesystem"
		return 1
	fi

	e2fsck -p $EXT4_DEV &> /dev/null
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "fsck returned failure"
		return 1
	fi

	tst_resm TPASS "ext4 persistent preallocation test pass"
}

# main
ext4_setup

RET=0

ext4_test_persist_prealloc 1024
if [ $? -ne 0 ]; then
	RET=1;
fi
: $((TST_COUNT++))

ext4_test_persist_prealloc 4096
if [ $? -ne 0 ]; then
	RET=1;
fi
: $((TST_COUNT++))

ext4_cleanup

exit $RET

