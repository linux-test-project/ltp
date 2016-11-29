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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
## Author: Li Zefan <lizf@cn.fujitsu.com>                                     ##
##         Miao Xie <miaox@cn.fujitsu.com>                                    ##
##                                                                            ##
################################################################################


export TCID="ext4-subdir-limit"
export TST_TOTAL=10

. ext4_funcs.sh

SHORT_DIR=1
LONG_DIR=2

FAIL=1
PASS=0

prev_block_size=-1
prev_result=$FAIL

# Run a test case
# $1: Number of directories to create
# $2: create short dir or long dir
# $3: parent directory
# $4: filesystem block size
ext4_run_case()
{
	local dir_name_len=

	if [ $2 -eq $SHORT_DIR ]; then
		dir_name_len="short name"
	else
		dir_name_len="long name"
	fi

	tst_resm TINFO "Num of dirs to create: $1, Dir name len: $dir_name_len, " \
			"Parent dir: $3, Block size: $4"

	# only mkfs if block size has been changed,
	# or previous case failed
	if [ $prev_result -ne $PASS -o $4 -ne $prev_block_size ]; then
		mkfs.ext4 -b $4 -I 256 $EXT4_DEV >/dev/null 2>&1
		if [ $? -ne 0 ]; then
			tst_resm TFAIL "failed to create ext4 filesystem"
			return
		fi
		prev_block_size=$4

		tune2fs -O extents $EXT4_DEV >/dev/null 2>&1
	fi

	prev_result=$FAIL

	# mount ext4 filesystem
	mount -t ext4 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount ext4 filesystem"
		return
	fi

	# create directories
	mkdir -p $3 2> /dev/null

	if [ $2 -eq $SHORT_DIR ]; then
		create_short_dirs $1 $3
	else
		create_long_dirs $1 $3
	fi

	if [ $? -ne 0 ]; then
		nr_dirs=`ls $3 | wc -l`
		tst_resm TFAIL "failed to create directories - $nr_dirs"
		umount mnt_point
		return
	fi

	# delete directories
	cd $3
	ls | xargs rmdir
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to remove directories"
		cd -
		umount mnt_point
		return
	fi
	cd -

	# unmount ext4 filesystem
	umount mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to umount ext4 filesystem"
		return
	fi

	# run fsck to make sure the filesystem has no errors
	e2fsck -p $EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "fsck: the filesystem has errors"
		return
	fi

	# check dir_nlink is set
	dumpe2fs $EXT4_DEV 2> /dev/null | grep '^Filesystem features' | grep -q dir_nlink
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "feature dir_nlink is not set"
		return
	fi

	prev_result=$PASS
	tst_resm TPASS "ext4 subdir limit test pass"
}

# main
ext4_setup

DIR_LEN=( $SHORT_DIR $LONG_DIR )
PARENT_DIR=( "mnt_point" "mnt_point/sub" )
BLOCK_SIZE=( 1024 2048 4096 )

for ((i = 0; i < 3; i++))
{
	for ((j = 0; j < 2; j++))
	{
		for ((k = 0; k < 2; k++))
		{
			if [ ${DIR_LEN[$k]} -eq $LONG_DIR -a \
				${BLOCK_SIZE[$i]} -eq 1024 ]; then
				continue
			fi
			ext4_run_case 65537 ${DIR_LEN[$k]} ${PARENT_DIR[$j]} \
					${BLOCK_SIZE[$i]}
		}
	}
}

tst_exit
