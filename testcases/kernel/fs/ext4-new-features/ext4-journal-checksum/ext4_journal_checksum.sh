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


export TCID="ext4-journal-checksum"
export TST_TOTAL=36

. ext4_funcs.sh

# Use ffsb to test journal checksumming
# $1: journal mode: writeback, ordered, journal
# $2: commit interval: 1 sec, 100 sec
# $3: journal_checksum or not
# $4: journal_async_commit or not
# $5: barrier: 0, 1
ext4_test_journal_checksum()
{
	local checksum=
	local async_commit=

	if [ -z "$3" ]; then
		checksum="No use"
	else
		checksum="Used"
	fi
	if [ -z "$4" ]; then
		async_commit="No use"
	else
		async_commit="Used"
	fi

	tst_resm TINFO "journal mode: $1, commit interval: $2, " \
		"journal_checksum: $checksum, " \
		"journal_async_commit: $async_commit, barrier: $5"

	mkfs.ext4 -I 256 $EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to create ext4 filesystem"
		return
	fi

	tune2fs -O extents $EXT4_DEV >/dev/null 2>&1

	mount -t ext4 -o data=$1,commit=$2,$3,$4,barrier=$5 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount ext4 filesystem"
		return
	fi

	ffsb ffsb-config2 > /dev/null
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

	e2fsck -p $EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "fsck returned failure"
		return
	fi

	tst_resm TPASS "ext4 journal checksum test pass"
}

# main
ext4_setup

tst_test_cmds ffsb

DATA=( "writeback" "ordered" "journal" )
COMMIT=( 1 100 )
JOURNAL_CHECKSUM=( "journal_checksum" "" )
JOURNAL_ASYNC_COMMIT=( "journal_async_commit" "" )
BARRIER=( 0 1 )

for ((i = 0; i < 2; i++))
{
	for ((j = 0; j < 2; j++))
	{
		for ((k = 0; k < 2; k++))
		{
			for ((l = 0; l < 2; l++))
			{
				for ((m = 0; m < 3; m++))
				{

					# when journal_async_commit is used,
					# journal_checksum is set automatically
					if [ $j -eq 0 -a $k -eq 1 ]; then
						continue
					fi

					ext4_test_journal_checksum ${DATA[$m]} \
						${COMMIT[$l]} \
						"${JOURNAL_CHECKSUM[$k]}" \
						"${JOURNAL_ASYNC_COMMIT[$j]}" \
						${BARRIER[$i]}
				}
			}
		}
	}
}

tst_exit
