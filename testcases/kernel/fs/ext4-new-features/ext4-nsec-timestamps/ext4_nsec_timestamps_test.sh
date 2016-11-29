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

export TCID="ext4-nsec-timestamps"
export TST_TOTAL=2

. ext4_funcs.sh

# Test that file timestamps is second with 128 inode size
ext4_test_sec_timestamps()
{
	tst_resm TINFO "Test timestamps with 128 inode size"

	mkfs.ext4 -I 128 $EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to create ext4 filesystem"
		return
	fi

	tune2fs -O extents $EXT4_DEV >/dev/null 2>&1

	mount -t ext4 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount ext4 filesystem"
		return
	fi

	touch mnt_point/tmp_file

	atime=`ext4_file_time mnt_point/tmp_file atime nsec`
	mtime=`ext4_file_time mnt_point/tmp_file mtime nsec`
	ctime=`ext4_file_time mnt_point/tmp_file ctime nsec`

	if [ $atime -ne 0 -o $mtime -ne 0 -o $ctime -ne 0 ]; then
		tst_resm TFAIL "Timestamp is not second(atime: $atime, mtime: \
				$mtime, ctime: $ctime)"
		umount mnt_point
		return
	fi

	umount mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to umount ext4 filesystem"
		return
	fi

	tst_resm TPASS "Ext4 nanosecond timestamps test with 128 inode size pass"
}

# Test file timestamps is nanosecond with 256 inode size
ext4_test_nsec_timestamps()
{
	tst_resm TINFO "Test timestamps with 256 inode size"

	mkfs.ext3 -I 256 $EXT4_DEV >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to create ext4 filesystem"
		return
	fi

	mount -t ext4 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount ext4 filesystem"
		return
	fi

	# Create file
	touch mnt_point/tmp_file
	sleep 1

	# Change atime, ctime and mtime of the file
	touch mnt_point/tmp_file

	cur_time=`date '+%s %N'`
	sec=`echo $cur_time | awk {'print $1'}`
	nsec=`echo $cur_time | awk {'print $2'}`

	sec_atime=`ext4_file_time mnt_point/tmp_file atime sec`
	sec_mtime=`ext4_file_time mnt_point/tmp_file mtime sec`
	sec_ctime=`ext4_file_time mnt_point/tmp_file ctime sec`
	nsec_atime=`ext4_file_time mnt_point/tmp_file atime nsec`
	nsec_mtime=`ext4_file_time mnt_point/tmp_file mtime nsec`
	nsec_ctime=`ext4_file_time mnt_point/tmp_file ctime nsec`

	# Test nanosecond
	if [ $nsec_atime -eq 0 -a $nsec_mtime -eq 0 -a $nsec_ctime -eq 0 ]
	then
		tst_resm TFAIL "The timestamp is not nanosecond(nsec_atime: $nsec_atime, nsec_mtime: $nsec_mtime, nsec_ctime: $nsec_ctime)"
		umount mnt_point
		return
	fi

	diff1=$(( $sec_atime - $sec ))
	diff2=$(( $sec_mtime - $sec ))
	diff3=$(( $sec_ctime - $sec ))

	# Test difference between file time and current time
	if [ $diff1 -gt 1 -o $diff2 -gt 1 -o $diff2 -gt 1 ]; then
		tst_resm TFAIL "The timestamp is wrong, it must be earlier \
			than the current time we got.(sec_atime: $sec_atime, \
			sec_mtime: $sec_mtime, sec_ctime: $sec_ctime, \
			cur_time[s]: $sec)"
		umount mnt_point
		return
	fi

	umount mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to umount ext4 filesystem"
		return
	fi

	# Test mount to ext3 and then mount back to ext4
	mount -t ext3 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount to ext3"
		return
	fi
	umount mnt_point

	mount -t ext4 $EXT4_DEV mnt_point
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount back to ext4"
		return
	fi

	nsec_atime2=`ext4_file_time mnt_point/tmp_file atime nsec`
	nsec_mtime2=`ext4_file_time mnt_point/tmp_file mtime nsec`
	nsec_ctime2=`ext4_file_time mnt_point/tmp_file mtime nsec`

	if [ $nsec_atime -ne $nsec_atime2 -o $nsec_ctime -ne $nsec_ctime2 -o \
	     $nsec_mtime -ne $nsec_mtime2 ]; then
		tst_resm TFAIL "File nanosecond timestamp has changed \
			unexpected. Before[atime mtime ctime]: $nsec_atime \
			$nsec_mtime $nsec_ctime, After[atime mtime ctime]: \
			$nsec_atime2 $nsec_mtime2 $nsec_ctime2)"
		umount mnt_point
		return
	fi

	umount mnt_point
	tst_resm TPASS "Ext4 nanosecond timestamps test with 256 inode size pass"
}

# main
ext4_setup

ext4_test_sec_timestamps
ext4_test_nsec_timestamps

tst_exit
