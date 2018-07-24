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

export TCID="ext4-uninit-groups"
export TST_TOTAL=24

. ext4_funcs.sh

# How to age filesystem
EMPTY=1
SMALL=2
LARGE=3

age_filesystem()
{
	if [ $1 -eq $EMPTY ]; then
		# aging, then del
		ffsb ffsb-config3 > /dev/null
		rm -rf mnt_point/*
	elif [ $1 -eq $SMALL ]; then
		# age filesystem from 0.0 to 0.2 -> 0.4 -> 0.6 -> 0.8 -> 1.0
		for ((n = 3; n < 8; n++))
		{
			ffsb ffsb-config$n > /dev/null
			mv mnt_point/data mnt_point/data$n
		}
	elif [ $1 -eq $LARGE ]; then
		rm -rf mnt_point/*
		bsize=`dumpe2fs -h $EXT4_DEV 2> /dev/null | grep 'Block size'`
		bsize=`echo $bsize | awk '{ print $3 }'`
		bcount=`dumpe2fs -h $EXT4_DEV 2> /dev/null | grep 'Free blocks'`
		bcount=`echo $bcount | awk '{ print $3 }'`
		dd if=/dev/zero of=mnt_point/tmp_dir bs=$bsize count=$bcount
	else
		return 1
	fi

	return 0
}

# Test uninitialized groups
# $1: orlov, oldalloc
# $2: delalloc
# $3: flex_bg
# $4: age filesystem: $EMPTY, $SMALL, $LARGE
ext4_test_uninit_groups()
{
	echo "Test $TST_COUNT" >> ext4_uninit_groups_result.txt

	mkfs.ext4 -I 256 -m 0 $EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to create ext4 filesystem"
		return
	fi

	if [ "$3" = "no_flex_bg" ]; then
		flag=""
	else
		flag=$3
	fi

	tune2fs -O extents,uninit_groups,$flag $EXT4_DEV >/dev/null 2>&1

	# Must run fsck after setting uninit_groups
	fsck -p $EXT4_DEV >/dev/null 2>&1

	mount -t ext4 -o $1,$2 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount ext4 filesystem"
		return
	fi

	age_filesystem $4 >> ext4_uninit_groups_result.txt 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "age filesystem failed"
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

	tst_resm TPASS "ext4 uninit groups test pass"
}

# main
ext4_setup

tst_test_cmds ffsb

ORLOV=( "orlov" "oldalloc" )
DELALLOC=( "delalloc" "nodelalloc" )
FLEX_BG=( "flex_bg" "no_flex_bg" )
AGING=( $EMPTY $SMALL $LARGE )


for ((i = 0; i < 2; i++))
{
	for ((j = 0; j < 2; j++))
	{
		for ((k = 0; k < 2; k++))
		{
			for ((l = 0; l < 3; l++))
			{
				ext4_test_uninit_groups ${ORLOV[$i]} \
							${DELALLOC[$j]} \
							${FLEX_BG[$k]} \
							${AGING[$l]}
			}
		}
	}
}

tst_exit
