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

export TCID="ext4-inode-version"
export TST_TOTAL=8
export TST_COUNT=1

export TEST_DIR=$PWD

# $1: the test config
read_config $1

# Test that inode version is not 32 bits with 128 inode size
ext4_test_128_inode_version()
{
	tst_resm TINFO "Test inode version is 32 bits with 128 inode size"

	mkfs.ext4 -I 128 $EXT4_DEV &> /dev/null
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to create ext4 filesystem"
		return 1
	fi

	tune2fs -O extents $EXT4_DEV &> /dev/null

	mount -t ext4 -o i_version $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount ext4 filesystem"
		return 1
	fi

	# inode version > 0
	touch mnt_point/tmp_file
	sync
	version=`./ext4_get_inode_version.sh tmp_file`

	if [ $version -lt 1 ]; then
		tst_resm TFAIL "inode version is smaller than 1"
		umount mnt_point
		return 1
	fi

	# inode version is 32 bits: 0x00000000
	version=`debugfs $EXT4_DEV -R "stat tmp_file" 2> /dev/null | grep 'Version'`
	version=`echo $version | awk '{ print $NF }'`
	version=`echo $version | sed 's/^0x//'`
	len=${#version}

	if [ $len -ne 8 ]; then
		tst_resm TFAIL "inode version is not 32 bits"
		umount mnt_point
		return 1
	fi

	umount mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to umount ext4 filesystem"
		return 1
	fi

	tst_resm TPASS "32 bits inode version with 128 inode size test pass"
}

# Test file timestamps is nanosecond with 256 inode size
# $1: file operation
test_inode_version()
{
	mkfs.ext3 -I 256 $EXT4_DEV &> /dev/null
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to create ext4 filesystem"
		return 1
	fi

	mount -t ext4 -o i_version $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount ext4 filesystem"
		return 1
	fi

	# Check the inode version after some file operation
	old_version=`./ext4_test_inode_version $1 mnt_point/tmp_file tmp_file`
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "inode version is wrong"
		umount mnt_point
		return 1
	fi

	umount mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to umount ext4 filesystem"
		return 1
	fi

	# Test mount to ext3 and then mount back to ext4
	mount -t ext3 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount to ext3"
		return 1
	fi
	umount mnt_point

	mount -t ext4 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount back to ext4"
		return 1
	fi

	version=`./ext4_get_inode_version.sh tmp_file`
#	echo "remount: old - $old_version"
#	echo "remount: new - $version"
	if [ $old_version -ne $version ]; then
		tst_resm TFAIL "inode version has changed unexpected"
		umount mnt_point
		return 1
	else
		tst_resm TPASS "inode version with 256 inode size test pass"
		umount mnt_point
	fi
}

# main
ext4_setup

RET=0

ext4_test_128_inode_version
if [ $? -ne 0 ]; then
	RET=1
fi
: $((TST_COUNT++))

test_inode_version create
if [ $? -ne 0 ]; then
	RET=1
fi
: $((TST_COUNT++))

test_inode_version chmod
if [ $? -ne 0 ]; then
	RET=1
fi
: $((TST_COUNT++))

test_inode_version chown
if [ $? -ne 0 ]; then
	RET=1
fi
: $((TST_COUNT++))

test_inode_version read
if [ $? -ne 0 ]; then
	RET=1
fi
: $((TST_COUNT++))

test_inode_version write
if [ $? -ne 0 ]; then
	RET=1
fi
: $((TST_COUNT++))

test_inode_version mmap_read
if [ $? -ne 0 ]; then
	RET=1
fi
: $((TST_COUNT++))

test_inode_version mmap_write
if [ $? -ne 0 ]; then
	RET=1
fi
: $((TST_COUNT++))

ext4_cleanup

exit $RET

