#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Test creates several zram devices with different filesystems on them.
# It fills each device with zeros and checks that compression works.

TST_CNT=7
TST_TESTFUNC="do_test"
TST_NEEDS_CMDS="awk bc dd"
. zram_lib.sh

# Test will create the following number of zram devices:
dev_num=4
# This is a list of parameters for zram devices.
# Number of items must be equal to 'dev_num' parameter.
zram_max_streams="2 3 5 8"

FS_SIZE="402653184"
FS_TYPE="btrfs"

RAM_SIZE=$(awk '/MemTotal:/ {print $2}' /proc/meminfo)
if [ "$RAM_SIZE" -lt 1048576 ]; then
	tst_res TINFO "not enough space for Btrfs"
	FS_SIZE="26214400"
	FS_TYPE="ext2"
fi

# The zram sysfs node 'disksize' value can be either in bytes,
# or you can use mem suffixes. But in some old kernels, mem
# suffixes are not supported, for example, in RHEL6.6GA's kernel
# layer, it uses strict_strtoull() to parse disksize which does
# not support mem suffixes, in some newer kernels, they use
# memparse() which supports mem suffixes. So here we just use
# bytes to make sure everything works correctly.
zram_sizes="26214400 26214400 26214400 $FS_SIZE"
zram_mem_limits="25M 25M 25M $((FS_SIZE/1024/1024))M"
zram_filesystems="ext3 ext4 xfs $FS_TYPE"

zram_fill_fs()
{
	for i in $(seq 0 $(($dev_num - 1))); do
		tst_res TINFO "filling zram$i (it can take long time)"
		local b=0
		while true; do
			dd conv=notrunc if=/dev/zero of=zram${i}/file \
				oflag=append count=1 bs=1024 status=none \
				>/dev/null 2>err.txt || break
			b=$(($b + 1))
		done
		if [ $b -eq 0 ]; then
			[ -s err.txt ] && tst_res TWARN "dd error: $(cat err.txt)"
			tst_brk TBROK "cannot fill zram $i"
		fi
		tst_res TPASS "zram$i was filled with '$b' KB"

		if [ ! -f "/sys/block/zram$i/mm_stat" ]; then
			if [ $i -eq 0 ]; then
				tst_res TCONF "zram compression ratio test requires zram mm_stat sysfs file"
			fi

			continue
		fi

		local compr_size=`awk '{print $2}' "/sys/block/zram$i/mm_stat"`
		local v=$((100 * 1024 * $b / $compr_size))
		local r=`echo "scale=2; $v / 100 " | bc`

		if [ "$v" -lt 100 ]; then
			tst_res TFAIL "compression ratio: $r:1"
			break
		fi

		tst_res TPASS "compression ratio: $r:1"
	done
}

do_test()
{
	case $1 in
	 1) zram_max_streams;;
	 2) zram_compress_alg;;
	 3) zram_set_disksizes;;
	 4) zram_set_memlimit;;
	 5) zram_makefs;;
	 6) zram_mount;;
	 7) zram_fill_fs;;
	esac
}

tst_run
