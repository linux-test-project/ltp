#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

dev_makeswap=-1
dev_mounted=-1

trap tst_exit INT

zram_cleanup()
{
	tst_resm TINFO "zram cleanup"
	local i=
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

	tst_rmdir
}

zram_load()
{
	tst_resm TINFO "create '$dev_num' zram device(s)"
	modprobe zram num_devices=$dev_num || \
		tst_brkm TBROK "failed to insert zram module"

	dev_num_created=$(ls /dev/zram* | wc -w)

	if [ "$dev_num_created" -ne "$dev_num" ]; then
		tst_brkm TFAIL "unexpected num of devices: $dev_num_created"
	else
		tst_resm TPASS "test succeeded"
	fi

	tst_tmpdir
}

zram_max_streams()
{
	if tst_kvcmp -lt "3.15" -o -ge "4.7"; then
		tst_resm TCONF "The device attribute max_comp_streams was"\
		               "introduced in kernel 3.15 and deprecated in 4.7"
		return
	fi

	tst_resm TINFO "set max_comp_streams to zram device(s)"

	local i=0
	for max_s in $zram_max_streams; do
		local sys_path="/sys/block/zram${i}/max_comp_streams"
		echo $max_s > $sys_path || \
			tst_brkm TFAIL "failed to set '$max_s' to $sys_path"
		local max_streams=$(cat $sys_path)

		[ "$max_s" -ne "$max_streams" ] && \
			tst_brkm TFAIL "can't set max_streams '$max_s', get $max_stream"

		i=$(($i + 1))
		tst_resm TINFO "$sys_path = '$max_streams' ($i/$dev_num)"
	done

	tst_resm TPASS "test succeeded"
}

zram_compress_alg()
{
	if tst_kvcmp -lt "3.15"; then
		tst_resm TCONF "device attribute comp_algorithm is"\
			"introduced since kernel v3.15, the running kernel"\
			"does not support it"
		return
	fi

	tst_resm TINFO "test that we can set compression algorithm"

	local algs=$(cat /sys/block/zram0/comp_algorithm)
	tst_resm TINFO "supported algs: $algs"
	local i=0
	for alg in $zram_algs; do
		local sys_path="/sys/block/zram${i}/comp_algorithm"
		echo "$alg" >  $sys_path || \
			tst_brkm TFAIL "can't set '$alg' to $sys_path"
		i=$(($i + 1))
		tst_resm TINFO "$sys_path = '$alg' ($i/$dev_num)"
	done

	tst_resm TPASS "test succeeded"
}

zram_set_disksizes()
{
	tst_resm TINFO "set disk size to zram device(s)"
	local i=0
	for ds in $zram_sizes; do
		local sys_path="/sys/block/zram${i}/disksize"
		echo "$ds" >  $sys_path || \
			tst_brkm TFAIL "can't set '$ds' to $sys_path"

		i=$(($i + 1))
		tst_resm TINFO "$sys_path = '$ds' ($i/$dev_num)"
	done

	tst_resm TPASS "test succeeded"
}

zram_set_memlimit()
{
	if tst_kvcmp -lt "3.18"; then
		tst_resm TCONF "device attribute mem_limit is"\
			"introduced since kernel v3.18, the running kernel"\
			"does not support it"
		return
	fi

	tst_resm TINFO "set memory limit to zram device(s)"

	local i=0
	for ds in $zram_mem_limits; do
		local sys_path="/sys/block/zram${i}/mem_limit"
		echo "$ds" >  $sys_path || \
			tst_brkm TFAIL "can't set '$ds' to $sys_path"

		i=$(($i + 1))
		tst_resm TINFO "$sys_path = '$ds' ($i/$dev_num)"
	done

	tst_resm TPASS "test succeeded"
}

zram_makeswap()
{
	tst_resm TINFO "make swap with zram device(s)"
	tst_check_cmds mkswap swapon swapoff
	local i=0
	for i in $(seq 0 $(($dev_num - 1))); do
		mkswap /dev/zram$i > err.log 2>&1
		if [ $? -ne 0 ]; then
			cat err.log
			tst_brkm TFAIL "mkswap /dev/zram$1 failed"
		fi

		swapon /dev/zram$i > err.log 2>&1
		if [ $? -ne 0 ]; then
			cat err.log
			tst_brkm TFAIL "swapon /dev/zram$1 failed"
		fi

		tst_resm TINFO "done with /dev/zram$i"
		dev_makeswap=$i
	done

	tst_resm TPASS "making zram swap succeeded"
}

zram_swapoff()
{
	tst_check_cmds swapoff
	local i=
	for i in $(seq 0 $dev_makeswap); do
		swapoff /dev/zram$i > err.log 2>&1
		if [ $? -ne 0 ]; then
			cat err.log
			tst_brkm TFAIL "swapoff /dev/zram$i failed"
		fi
	done
	dev_makeswap=-1

	tst_resm TPASS "swapoff completed"
}

zram_makefs()
{
	tst_check_cmds mkfs which
	local i=0
	for fs in $zram_filesystems; do
		# if requested fs not supported default it to ext2
		which mkfs.$fs > /dev/null 2>&1 || fs=ext2

		tst_resm TINFO "make $fs filesystem on /dev/zram$i"
		mkfs.$fs /dev/zram$i > err.log 2>&1
		if [ $? -ne 0 ]; then
			cat err.log
			tst_brkm TFAIL "failed to make $fs on /dev/zram$i"
		fi
		i=$(($i + 1))
	done

	tst_resm TPASS "zram_makefs succeeded"
}

zram_mount()
{
	local i=0
	for i in $(seq 0 $(($dev_num - 1))); do
		tst_resm TINFO "mount /dev/zram$i"
		mkdir zram$i
		mount /dev/zram$i zram$i > /dev/null || \
			tst_brkm TFAIL "mount /dev/zram$i failed"
		dev_mounted=$i
	done

	tst_resm TPASS "mount of zram device(s) succeeded"
}

modinfo zram > /dev/null 2>&1 ||
	tst_brkm TCONF "zram not configured in kernel"
