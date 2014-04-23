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

export TCID="ext4-online-defrag"
export TST_TOTAL=18

. ext4_funcs.sh

# How to age filesystem
EMPTY=1
SMALL=2
LARGE=3

# Defrag what
FILE=1
DIR=2
FILESYSTEM=3

tst_check_cmds e4defrag
E4DEFRAG=`which e4defrag`

age_filesystem()
{
	local dirId=
	local idx=
	rm -rf mnt_point/*
	if [ $1 -eq $EMPTY ]; then
		if [ $2 -eq $FILE -o $2 -eq $FILESYSTEM ]; then
			touch mnt_point/tmp_file
			echo "abc" > mnt_point/tmp_file
		elif [ $2 -eq $DIR ]; then
			mkdir mnt_point/tmp_dir
			echo "abc" > mnt_point/tmp_dir/tmp_file
		fi
	elif [ $1 -eq $SMALL ]; then

		dd if=/dev/zero of=mnt_point/occupy bs=1M count=40

		# age filesystem from 0.0 to 0.2 -> 0.4 -> 0.6 -> 0.8 -> 1.0
		for ((idx = 3; idx < 8; idx++))
		{
			ffsb ffsb-config$idx > /dev/null
			dirId=$((idx - 3))
			mv mnt_point/data mnt_point/data$dirId
		}

		rm mnt_point/occupy

		df
	else
		rm -rf mnt_point/*
		if [ $2 -eq $DIR ]; then
			mkdir mnt_point/tmp_dir
			dest=mnt_point/tmp_dir/tmp_file
		else
			dest=mnt_point/tmp_file
		fi

		bsize=`dumpe2fs -h $EXT4_DEV | grep 'Block size'`
		bsize=`echo $bsize | awk '{ print $NF }'`
		bcount=`dumpe2fs -h $EXT4_DEV | grep 'Free blocks'`
		bcount=`echo $bcount | awk '{ print $NF }'`
		bcount=$(( $bcount / 2 - 100 ))
		dd if=/dev/zero of=$dest bs=$bsize count=$bcount

	fi
}

my_e4defrag()
{
	if [ $1 -eq $FILE ]; then
		if [ $2 -eq $SMALL ]; then
			$E4DEFRAG -v mnt_point/data0/
			return $?
		# EMPTY or LARGE
		else
			$E4DEFRAG -v mnt_point/tmp_file
			return $?
		fi
	elif [ $1 -eq $DIR ]; then
		if [ $2 -eq $SMALL ]; then
			$E4DEFRAG -v mnt_point/data0/
			return $?
		else
			$E4DEFRAG -v mnt_point/tmp_dir
			return $?
		fi
	else
		$E4DEFRAG -v $EXT4_DEV
		return $?
	fi
}

# Test online defragmentation feature
# $1: defrag type
# $2: 1 - empty, 2 - full with small files, 3 - full with large files
# $3: block size
ext4_test_online_defrag()
{
	echo Test $TST_COUNT start >> ext4_online_defrag_result.txt

	tst_resm TINFO "defrag type: $1, defrag obj: $2, block size: $3"

	mkfs.ext4 -m 0 -b $3 -O ^flex_bg $EXT4_DEV >> ext4_online_defrag_result.txt 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to create ext4 filesystem"
		return
	fi

	tune2fs -O extents $EXT4_DEV >> ext4_online_defrag_result.txt 2>&1

	mount -t ext4 -o nodelalloc $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL"failed to mount ext4 filesystem"
		return
	fi

	age_filesystem $2 $1 >> ext4_online_defrag_result.txt 2>&1

	my_e4defrag $1 $2 >> ext4_online_defrag_result.txt
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "e4defrag returned failure"
		umount mnt_point
		return
	fi

	umount mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to umount ext4 filesystem"
		return
	fi

	e2fsck -p $EXT4_DEV >> ext4_online_defrag_result.txt 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "fsck returned failure"
		return
	fi

	tst_resm TPASS "ext4 online defrag test pass"
}

# main
ext4_setup

tst_check_cmds ffsb

DEFRAG=( $FILE $DIR $FILESYSTEM )
AGING=( $EMPTY $SMALL $LARGE )
BLOCK_SIZE=( 1024 4096 )

for ((i = 0; i < 2; i++))
{
	for ((j = 0; j < 3; j++))
	{
		for ((k = 0; k < 3; k++))
		{
			ext4_test_online_defrag ${DEFRAG[$j]} ${AGING[$k]} \
						${BLOCK_SIZE[$i]}
		}
	}
}

tst_exit
