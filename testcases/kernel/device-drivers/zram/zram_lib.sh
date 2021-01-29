#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2019-2021 Petr Vorel <pvorel@suse.cz>
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

dev_makeswap=-1
dev_mounted=-1

TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_SETUP="zram_load"
TST_CLEANUP="zram_cleanup"
TST_NEEDS_DRIVERS="zram"
. tst_test.sh

zram_cleanup()
{
	local i

	for i in $(seq 0 $dev_makeswap); do
		swapoff /dev/zram$i
	done

	for i in $(seq 0 $dev_mounted); do
		umount /dev/zram$i
	done

	for i in $(seq 0 $(($dev_num - 1))); do
		echo 1 > /sys/block/zram${i}/reset
	done

	rmmod zram > /dev/null 2>&1
}

zram_load()
{
	local tmp

	if [ -z "$dev_num" ]; then
		dev_num=0
		for tmp in $zram_max_streams; do
			dev_num=$((dev_num+1))
		done
	fi

	if [ $dev_num -le 0 ]; then
		tst_brk TBROK "dev_num must be > 0"
	fi

	tst_set_timeout $((dev_num*450))

	tst_res TINFO "create '$dev_num' zram device(s)"

	modprobe zram num_devices=$dev_num || \
		tst_brk TBROK "failed to insert zram module"

	dev_num_created=$(ls /dev/zram* | wc -w)

	if [ "$dev_num_created" -ne "$dev_num" ]; then
		tst_brk TFAIL "unexpected num of devices: $dev_num_created"
	fi

	tst_res TPASS "all zram devices successfully created"
}

zram_max_streams()
{
	if tst_kvcmp -lt "3.15" -o -ge "4.7"; then
		tst_res TCONF "The device attribute max_comp_streams was"\
		               "introduced in kernel 3.15 and deprecated in 4.7"
		return
	fi

	tst_res TINFO "set max_comp_streams to zram device(s)"

	local i=0

	for max_s in $zram_max_streams; do
		local sys_path="/sys/block/zram${i}/max_comp_streams"
		echo $max_s > $sys_path || \
			tst_brk TFAIL "failed to set '$max_s' to $sys_path"
		local max_streams=$(cat $sys_path)

		[ "$max_s" -ne "$max_streams" ] && \
			tst_brk TFAIL "can't set max_streams '$max_s', get $max_stream"

		i=$(($i + 1))
		tst_res TINFO "$sys_path = '$max_streams' ($i/$dev_num)"
	done

	tst_res TPASS "test succeeded"
}

zram_compress_alg()
{
	if tst_kvcmp -lt "3.15"; then
		tst_res TCONF "device attribute comp_algorithm is"\
			"introduced since kernel v3.15, the running kernel"\
			"does not support it"
		return
	fi

	local i=0

	tst_res TINFO "test that we can set compression algorithm"
	local algs="$(sed 's/[][]//g' /sys/block/zram0/comp_algorithm)"
	tst_res TINFO "supported algs: $algs"

	local dev_max=$(($dev_num - 1))

	for i in $(seq 0 $dev_max); do
		for alg in $algs; do
			local sys_path="/sys/block/zram${i}/comp_algorithm"
			echo "$alg" >  $sys_path || \
				tst_brk TFAIL "can't set '$alg' to $sys_path"
			tst_res TINFO "$sys_path = '$alg' ($i/$dev_max)"
		done
	done

	tst_res TPASS "test succeeded"
}

zram_set_disksizes()
{
	local i=0
	local ds

	tst_res TINFO "set disk size to zram device(s)"
	for ds in $zram_sizes; do
		local sys_path="/sys/block/zram${i}/disksize"
		echo "$ds" >  $sys_path || \
			tst_brk TFAIL "can't set '$ds' to $sys_path"

		i=$(($i + 1))
		tst_res TINFO "$sys_path = '$ds' ($i/$dev_num)"
	done

	tst_res TPASS "test succeeded"
}

zram_set_memlimit()
{
	if tst_kvcmp -lt "3.18"; then
		tst_res TCONF "device attribute mem_limit is"\
			"introduced since kernel v3.18, the running kernel"\
			"does not support it"
		return
	fi

	local i=0
	local ds

	tst_res TINFO "set memory limit to zram device(s)"

	for ds in $zram_mem_limits; do
		local sys_path="/sys/block/zram${i}/mem_limit"
		echo "$ds" >  $sys_path || \
			tst_brk TFAIL "can't set '$ds' to $sys_path"

		i=$(($i + 1))
		tst_res TINFO "$sys_path = '$ds' ($i/$dev_num)"
	done

	tst_res TPASS "test succeeded"
}
