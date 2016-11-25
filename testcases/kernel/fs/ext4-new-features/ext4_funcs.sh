#!/bin/sh

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

. test.sh

ext4_setup()
{
	if tst_kvcmp -lt "2.6.31"; then
		tst_brkm TCONF "kernel is below 2.6.31"
	fi

	tst_require_root

	EXT4_KERNEL_SUPPORT=`grep -w ext4 /proc/filesystems | cut -f2`
	if [ "$EXT4_KERNEL_SUPPORT" != "ext4" ]; then
		modprobe ext4 > /dev/null 2>&1
		if [ $? -ne 0 ]; then
			tst_brkm TCONF "Ext4 is not supported"
		fi
	fi

	if [ -z "$LTP_BIG_DEV" ];then
		tst_brkm TCONF "tests need a big block device(5G-10G)"
	else
		export EXT4_DEV=$LTP_BIG_DEV
	fi

	tst_tmpdir
	TST_CLEANUP=ext4_cleanup

	mkdir mnt_point
}

ext4_cleanup()
{
	rm -rf mnt_point
	tst_rmdir
}
