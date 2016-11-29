#!/bin/bash

################################################################################
#                                                                              #
# Copyright (c) 2009 FUJITSU LIMITED                                           #
#                                                                              #
# This program is free software;  you can redistribute it and#or modify        #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful, but          #
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY   #
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License     #
# for more details.                                                            #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program;  if not, write to the Free Software                 #
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA      #
#                                                                              #
################################################################################

export TCID="ext4-delalloc-mballoc"
export TST_TOTAL=17

. ext4_funcs.sh

# Case 17: mount ext4 partition to ext3
ext4_test_remount()
{
	mkfs.ext3 -I 256 -b 1024 $EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to create ext4 filesystem"
		return
	fi

	mount -t ext4 -o delalloc,auto_da_alloc $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount ext4 filesystem"
		return
	fi

	ffsb $LTPROOT/testcases/data/ext4-ffsb/ffsb-config0 > /dev/null
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "ffsb returned failure"
		umount mnt_point
		return
	fi

	umount mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to umount ext4 filesystem"
		return
	fi

	mount -t ext3 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount to ext3"
		return
	fi
	umount mnt_point

	fsck -p $EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "fsck returned failure"
		return
	fi

	tst_resm TPASS "remount test pass"
}

# Case 1-16: Use ffsb to test mballoc and delalloc
# $1: delalloc or nodelalloc
# $2: 0 - indirect-io, 1 - direct-io
# $3: block size
# $4: auto_da_alloc
ext4_test_delalloc_mballoc()
{
	tst_resm TINFO "isDelalloc: $1, isDirectIO: $2, Blocksize: $3, isAuto_da_alloc: $4"

	mkfs.ext4 -I 256 -b $3 /$EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to create ext4 filesystem"
		return
	fi

	tune2fs -O extents $EXT4_DEV >/dev/null 2>&1

	mount -t ext4 -o $1,$4 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount ext4 filesystem"
		return
	fi

	ffsb $LTPROOT/testcases/data/ext4-ffsb/ffsb-config$2 > /dev/null
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "ffsb returned failure"
		umount mnt_point
		return
	fi

	umount mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to umount ext4 filesystem"
		return
	fi

	fsck -p $EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "fsck returned failure"
		return
	fi

	tst_resm TPASS "delalloc/mballoc test pass"
}

# main
ext4_setup

tst_check_cmds ffsb

DELALLOC=( "delalloc" "nodelalloc" )
DIRECT_IO=( 0 1 )
BLOCK_SIZE=( 1024 4096 )
BLOCK_AUTO_DA_ALLOC=( "auto_da_alloc=1" "noauto_da_alloc" )

for ((i = 0; i < 2; i++))
{
	for ((j = 0; j < 2; j++))
	{
		for ((k = 0; k < 2; k++))
		{
			for ((l = 0; l < 2; l++))
			{
				ext4_test_delalloc_mballoc ${DELALLOC[$k]} \
						${DIRECT_IO[$j]} \
						${BLOCK_SIZE[$i]} \
						${BLOCK_AUTO_DA_ALLOC[$l]}
			}
		}
	}
}

ext4_test_remount

tst_exit
