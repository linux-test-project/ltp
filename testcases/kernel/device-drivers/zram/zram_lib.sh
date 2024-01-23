#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) 2019-2022 Petr Vorel <pvorel@suse.cz>
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

dev_makeswap=-1
dev_mounted=-1
dev_start=0
dev_end=-1
module_load=-1
sys_control=-1

TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_SETUP="${TST_SETUP:-zram_load}"
TST_CLEANUP="${TST_CLEANUP:-zram_cleanup}"
TST_NEEDS_DRIVERS="zram"

zram_cleanup()
{
	local i

	for i in $(seq $dev_start $dev_makeswap); do
		swapoff /dev/zram$i
	done

	for i in $(seq $dev_start $dev_mounted); do
		umount /dev/zram$i
	done

	for i in $(seq $dev_start $dev_end); do
		echo 1 > /sys/block/zram${i}/reset
	done

	if [ $sys_control -eq 1 ]; then
		for i in $(seq $dev_start $dev_end); do
			echo $i > /sys/class/zram-control/hot_remove
		done
	fi

	if [ $module_load -eq 1 ]; then
		rmmod zram > /dev/null 2>&1
	fi
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

	# zram module loaded, new kernel
	if [ -d "/sys/class/zram-control" ]; then
		tst_res TINFO "zram module already loaded, kernel supports zram-control interface"
		dev_start=$(ls /dev/zram* | wc -w)
		dev_end=$(($dev_start + $dev_num - 1))
		sys_control=1

		for i in $(seq  $dev_start $dev_end); do
			cat /sys/class/zram-control/hot_add > /dev/null
		done

		tst_res TPASS "all zram devices (/dev/zram$dev_start~$dev_end) successfully created"
		return
	fi

	# detect old kernel or built-in
	modprobe zram num_devices=$dev_num
	if [ ! -d "/sys/class/zram-control" ]; then
		if grep -q '^zram' /proc/modules; then
			rmmod zram > /dev/null 2>&1 || \
				tst_brk TCONF "zram module is being used on old kernel without zram-control interface"
		else
			tst_brk TCONF "test needs CONFIG_ZRAM=m on old kernel without zram-control interface"
		fi
		modprobe zram num_devices=$dev_num
	fi

	module_load=1
	dev_end=$(($dev_num - 1))
	tst_res TPASS "all zram devices (/dev/zram0~$dev_end) successfully created"
}

zram_max_streams()
{
	if tst_kvcmp -lt "3.15" -o -ge "4.7"; then
		tst_res TCONF "The device attribute max_comp_streams was"\
		               "introduced in kernel 3.15 and deprecated in 4.7"
		return
	fi

	tst_res TINFO "set max_comp_streams to zram device(s)"

	local i=$dev_start

	for max_s in $zram_max_streams; do
		local sys_path="/sys/block/zram${i}/max_comp_streams"
		if ! echo $max_s > $sys_path; then
			tst_res TFAIL "failed to set '$max_s' to $sys_path"
			return
		fi
		local max_streams=$(cat $sys_path)

		if [ "$max_s" -ne "$max_streams" ]; then
			tst_res TFAIL "can't set max_streams '$max_s', get $max_stream"
			return
		fi

		i=$(($i + 1))
		tst_res TINFO "$sys_path = '$max_streams'"
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

	local i=$dev_start

	tst_res TINFO "test that we can set compression algorithm"
	local algs="$(sed 's/[][]//g' /sys/block/zram${i}/comp_algorithm)"
	tst_res TINFO "supported algs: $algs"

	for i in $(seq $dev_start $dev_end); do
		for alg in $algs; do
			local sys_path="/sys/block/zram${i}/comp_algorithm"
			if ! echo "$alg" >  $sys_path; then
				tst_res TFAIL "can't set '$alg' to $sys_path"
				return
			fi
			tst_res TINFO "$sys_path = '$alg'"
		done
	done

	tst_res TPASS "test succeeded"
}

zram_set_disksizes()
{
	local i=$dev_start
	local ds

	tst_res TINFO "set disk size to zram device(s)"
	for ds in $zram_sizes; do
		local sys_path="/sys/block/zram${i}/disksize"
		if ! echo "$ds" >  $sys_path; then
			tst_res TFAIL "can't set '$ds' to $sys_path"
			return
		fi

		i=$(($i + 1))
		tst_res TINFO "$sys_path = '$ds'"
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

	local i=$dev_start
	local ds

	tst_res TINFO "set memory limit to zram device(s)"

	for ds in $zram_mem_limits; do
		local sys_path="/sys/block/zram${i}/mem_limit"
		if ! echo "$ds" >  $sys_path; then
			tst_res TFAIL "can't set '$ds' to $sys_path"
			return
		fi

		i=$(($i + 1))
		tst_res TINFO "$sys_path = '$ds'"
	done

	tst_res TPASS "test succeeded"
}

. tst_test.sh
