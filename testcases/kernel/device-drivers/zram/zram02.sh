#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2019-2021 Petr Vorel <pvorel@suse.cz>
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Test checks that we can create swap zram device.

TST_CNT=6
TST_TESTFUNC="do_test"
. zram_lib.sh

# List of parameters for zram devices.
# For each number the test creates own zram device.
zram_max_streams="2"

# The zram sysfs node 'disksize' value can be either in bytes,
# or you can use mem suffixes. But in some old kernels, mem
# suffixes are not supported, for example, in RHEL6.6GA's kernel
# layer, it uses strict_strtoull() to parse disksize which does
# not support mem suffixes, in some newer kernels, they use
# memparse() which supports mem suffixes. So here we just use
# bytes to make sure everything works correctly.
zram_sizes="107374182400" # 100GB
zram_mem_limits="1M"

zram_makeswap()
{
	tst_res TINFO "make swap with zram device(s)"
	tst_require_cmds mkswap swapon swapoff
	local i=0

	for i in $(seq $dev_start $dev_end); do
		ROD mkswap /dev/zram$i
		ROD swapon /dev/zram$i
		tst_res TINFO "done with /dev/zram$i"
		dev_makeswap=$i
	done

	tst_res TPASS "making zram swap succeeded"
}

zram_swapoff()
{
	tst_require_cmds swapoff
	local i

	for i in $(seq $dev_start $dev_end); do
		ROD swapoff /dev/zram$i
	done
	dev_makeswap=-1

	tst_res TPASS "swapoff completed"
}

do_test()
{
	case $1 in
	 1) zram_max_streams;;
	 2) zram_compress_alg;;
	 3) zram_set_disksizes;;
	 4) zram_set_memlimit;;
	 5) zram_makeswap;;
	 6) zram_swapoff;;
	esac
}

tst_run
