#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2019-2022 Petr Vorel <pvorel@suse.cz>
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Test creates several zram devices with different filesystems on them.
# It fills each device with zeros and checks that compression works.

TST_CNT=7
TST_TESTFUNC="do_test"
TST_NEEDS_CMDS="awk bc dd"
TST_SETUP="setup"

check_space_for_fs()
{
	local fs="$1"
	local ram_size

	ram_size=$(awk '/MemTotal:/ {print $2}' /proc/meminfo)
	if [ "$ram_size" -lt 1048576 ]; then
		tst_res TINFO "not enough space for $fs"
		return 1
	fi
	return 0
}

# List of parameters for zram devices.
# For each number the test creates own zram device.
# NOTE about size:
# The zram sysfs node 'disksize' value can be either in bytes,
# or you can use mem suffixes. But in some old kernels, mem
# suffixes are not supported, for example, in RHEL6.6GA's kernel
# layer, it uses strict_strtoull() to parse disksize which does
# not support mem suffixes, in some newer kernels, they use
# memparse() which supports mem suffixes. So here we just use
# bytes to make sure everything works correctly.
initialize_vars()
{
	local fs limit size stream=-1
	dev_num=0

	for fs in $(tst_supported_fs -s tmpfs); do
		size="26214400"
		limit="25M"

		if [ "$fs" = "btrfs" -o "$fs" = "xfs" ]; then
			check_space_for_fs "$fs" || continue

			if [ "$fs" = "btrfs" ]; then
				size="402653184"
			elif [ "$fs" = "xfs" ]; then
				size=314572800
			fi
			limit="$((size/1024/1024))M"
		fi

		stream=$((stream+3))
		dev_num=$((dev_num+1))
		zram_filesystems="$zram_filesystems $fs"
		zram_mem_limits="$zram_mem_limits $limit"
		zram_sizes="$zram_sizes $size"
		zram_max_streams="$zram_max_streams $stream"
	done

	[ $dev_num -eq 0 ] && \
		tst_brk TCONF "no suitable filesystem"
}

setup()
{
	initialize_vars
	zram_load
}

zram_makefs()
{
	local i=$dev_start
	local fs

	for fs in $zram_filesystems; do
		tst_res TINFO "make $fs filesystem on /dev/zram$i"
		mkfs.$fs /dev/zram$i > err.log 2>&1
		if [ $? -ne 0 ]; then
			cat err.log
			tst_brk TFAIL "failed to make $fs on /dev/zram$i"
		fi

		i=$(($i + 1))
	done

	tst_res TPASS "zram_makefs succeeded"
}

zram_mount()
{
	local i=0

	for i in $(seq $dev_start $dev_end); do
		tst_res TINFO "mount /dev/zram$i"
		mkdir zram$i
		ROD mount /dev/zram$i zram$i
		dev_mounted=$i
	done

	tst_res TPASS "mount of zram device(s) succeeded"
}

zram_fill_fs()
{
	for i in $(seq $dev_start $dev_end); do
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

		local mem_used_total=`awk '{print $3}' "/sys/block/zram$i/mm_stat"`
		local v=$((100 * 1024 * $b / $mem_used_total))
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

. zram_lib.sh
tst_run
