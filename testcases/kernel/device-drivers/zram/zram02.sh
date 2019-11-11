#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Test checks that we can create swap zram device.

TST_CNT=5
TST_TESTFUNC="do_test"
. zram_lib.sh

# Test will create the following number of zram devices:
dev_num=1
# This is a list of parameters for zram devices.
# Number of items must be equal to 'dev_num' parameter.
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

do_test()
{
	case $1 in
	 1) zram_max_streams;;
	 2) zram_set_disksizes;;
	 3) zram_set_memlimit;;
	 4) zram_makeswap;;
	 5) zram_swapoff;;
	esac
}

tst_run
