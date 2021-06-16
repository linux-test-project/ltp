#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2012 FUJITSU LIMITED
# Copyright (c) 2014-2019 Linux Test Project
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
#
# Author: Peng Haitao <penght@cn.fujitsu.com>

TST_NEEDS_CHECKPOINTS=1
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="killall find kill"
TST_CLEANUP=memcg_cleanup
TST_SETUP=memcg_setup
TST_TESTFUNC=memcg_testfunc

MEMCG_SHMMAX=${MEMCG_SHMMAX:-0}
MEMCG_TESTFUNC=${MEMCG_TESTFUNC:-memcg_no_testfunc}

. cgroup_lib.sh

PAGESIZE=$(tst_getconf PAGESIZE)
if [ $? -ne 0 ]; then
	tst_brk TBROK "tst_getconf PAGESIZE failed"
fi

# Post 4.16 kernel updates stat in batch (> 32 pages) every time
PAGESIZES=$(($PAGESIZE * 33))

HUGEPAGESIZE=$(awk '/Hugepagesize/ {print $2}' /proc/meminfo)
[ -z $HUGEPAGESIZE ] && HUGEPAGESIZE=0
HUGEPAGESIZE=$(($HUGEPAGESIZE * 1024))

orig_memory_use_hierarchy=
orig_shmmax=

memcg_require_memsw()
{
	if ! [ -e /dev/memcg/memory.limit_in_bytes ]; then
		tst_brk TBROK "/dev/memcg must be mounted before calling memcg_require_memsw"
	fi
	if ! [ -e /dev/memcg/memory.memsw.limit_in_bytes ]; then
		tst_brk TCONF "mem+swap is not enabled"
	fi
}

memcg_require_hierarchy_disabled()
{
	if [ ! -e "/dev/memcg/memory.use_hierarchy" ]; then
		tst_brk TBROK "/dev/memcg must be mounted before calling memcg_require_hierarchy_disabled"
	fi
	if [ $(cat /dev/memcg/memory.use_hierarchy) -eq 1 ]; then
		tst_brk TCONF "Test requires root cgroup memory.use_hierarchy=0"
	fi
}

memcg_setup()
{
	if ! is_cgroup_subsystem_available_and_enabled "memory"; then
		tst_brk TCONF "Either kernel does not support Memory Resource Controller or feature not enabled"
	fi

	# Setup IPC
	LTP_IPC_PATH="/dev/shm/ltp_${TCID}_$$"
	LTP_IPC_SIZE=$PAGESIZE
	ROD_SILENT dd if=/dev/zero of="$LTP_IPC_PATH" bs="$LTP_IPC_SIZE" count=1
	ROD_SILENT chmod 600 "$LTP_IPC_PATH"
	export LTP_IPC_PATH
	# Setup IPC end

	ROD mkdir /dev/memcg
	ROD mount -t cgroup -omemory memcg /dev/memcg

	# The default value for memory.use_hierarchy is 0 and some of tests
	# (memcg_stat_test.sh and memcg_use_hierarchy_test.sh) expect it so
	# while there are distributions (RHEL7U0Beta for example) that sets
	# it to 1.
	# Note: If there are already subgroups created it is not possible,
	# to set this back to 0.
	# This seems to be the default for all systems using systemd.
	orig_memory_use_hierarchy=$(cat /dev/memcg/memory.use_hierarchy)
	if [ -z "$orig_memory_use_hierarchy" ];then
		tst_res TINFO "cat /dev/memcg/ failed"
	elif [ "$orig_memory_use_hierarchy" = "0" ];then
		orig_memory_use_hierarchy=""
	else
		echo 0 > /dev/memcg/memory.use_hierarchy 2>/dev/null
		if [ $? -ne 0 ];then
			tst_res TINFO "set /dev/memcg/memory.use_hierarchy to 0 failed"
		fi
	fi

	[ "$MEMCG_SHMMAX" = "1" ] && shmmax_setup
}

memcg_cleanup()
{
	kill -9 $MEMCG_PROCESS_PID 2> /dev/null

	cd $TST_TMPDIR
	# In order to remove all subgroups, we have to remove them recursively
	if [ -e /dev/memcg/ltp_$$ ]; then
		ROD find /dev/memcg/ltp_$$ -depth -type d -delete
	fi

	if [ -n "$orig_memory_use_hierarchy" ];then
		echo $orig_memory_use_hierarchy > /dev/memcg/memory.use_hierarchy
		if [ $? -ne 0 ];then
			tst_res TINFO "restore /dev/memcg/memory.use_hierarchy failed"
		fi
		orig_memory_use_hierarchy=""
	fi

	if [ -e "/dev/memcg" ]; then
		umount /dev/memcg
		rmdir /dev/memcg
	fi

	[ "$MEMCG_SHMMAX" = "1" ] && shmmax_cleanup
}

shmmax_setup()
{
	tst_require_cmds bc

	tst_res TINFO "Setting shmmax"

	orig_shmmax=$(cat /proc/sys/kernel/shmmax)
	if [ $(echo "$orig_shmmax < $HUGEPAGESIZE" | bc) -eq 1 ]; then
		ROD echo "$HUGEPAGESIZE" \> /proc/sys/kernel/shmmax
	fi
}

shmmax_cleanup()
{
	if [ -n "$orig_shmmax" ]; then
		echo "$orig_shmmax" > /proc/sys/kernel/shmmax
	fi
}

# Check size in memcg
# $1 - Item name
# $2 - Expected size lower bound
# $3 - Expected size upper bound (optional)
check_mem_stat()
{
	local item_size

	if [ -e $1 ]; then
		item_size=$(cat $1)
	else
		item_size=$(grep -w $1 memory.stat | cut -d " " -f 2)
	fi

	if [ "$3" ]; then
		if [ $item_size -ge $2 ] && [ $item_size -le $3 ]; then
			tst_res TPASS "$1 is ${2}-${3} as expected"
		else
			tst_res TFAIL "$1 is $item_size, ${2}-${3} expected"
		fi
	elif [ "$2" = "$item_size" ]; then
		tst_res TPASS "$1 is $2 as expected"
	else
		tst_res TFAIL "$1 is $item_size, $2 expected"
	fi
}

start_memcg_process()
{
	tst_res TINFO "Running memcg_process $@"
	memcg_process "$@" &
	MEMCG_PROCESS_PID=$!
	ROD tst_checkpoint wait 10000 0
}

signal_memcg_process()
{
	local size=$1
	local path=$2
	local usage_start=$(cat ${path}memory.usage_in_bytes)

	kill -s USR1 $MEMCG_PROCESS_PID 2> /dev/null

	if [ -z "$size" ]; then
		return
	fi

	local loops=100

	while kill -0 $MEMCG_PROCESS_PID 2> /dev/null; do
		local usage=$(cat ${path}memory.usage_in_bytes)
		local diff_a=$((usage_start - usage))
		local diff_b=$((usage - usage_start))

		if [ "$diff_a" -ge "$size" -o "$diff_b" -ge "$size" ]; then
			return
		fi

		tst_sleep 100ms

		loops=$((loops - 1))
		if [ $loops -le 0 ]; then
			tst_brk TBROK "timed out on memory.usage_in_bytes"
		fi
	done
}

stop_memcg_process()
{
	[ -z "$MEMCG_PROCESS_PID" ] && return
	kill -s INT $MEMCG_PROCESS_PID 2> /dev/null
	wait $MEMCG_PROCESS_PID
	MEMCG_PROCESS_PID=
}

warmup()
{
	tst_res TINFO "Warming up pid: $MEMCG_PROCESS_PID"
	signal_memcg_process
	signal_memcg_process
	sleep 1

	if ! kill -0 $MEMCG_PROCESS_PID; then
		wait $MEMCG_PROCESS_PID
		tst_res TFAIL "Process $MEMCG_PROCESS_PID exited with $? after warm up"
		return 1
	else
		tst_res TINFO "Process is still here after warm up: $MEMCG_PROCESS_PID"
	fi

	return 0
}

# Run test cases which checks memory.stat after make
# some memory allocation
test_mem_stat()
{
	local memtypes="$1"
	local size=$2
	local total_size=$3
	local stat_name=$4
	local exp_stat_size=$5
	local check_after_free=$6

	start_memcg_process $memtypes -s $size

	if ! warmup; then
		return
	fi

	echo $MEMCG_PROCESS_PID > tasks
	signal_memcg_process $size

	check_mem_stat $stat_name $exp_stat_size

	signal_memcg_process $size
	if $check_after_free; then
		check_mem_stat $stat_name 0
	fi

	stop_memcg_process
}

# Test process will be killed due to exceed memory limit
# $1 - the value of memory.limit_in_bytes
# $2 - the parameters of 'process', such as --shm
# $3 - the -s parameter of 'process', such as 4096
# $4 - use mem+swap limitation
test_proc_kill()
{
	local limit=$1
	local memtypes="$2"
	local size=$3
	local use_memsw=$4
	local tpk_iter

	echo $limit > memory.limit_in_bytes
	if [ $use_memsw -eq 1 ]; then
		memcg_require_memsw
		echo $limit > memory.memsw.limit_in_bytes
	fi

	start_memcg_process $memtypes -s $size
	echo $MEMCG_PROCESS_PID > tasks

	signal_memcg_process $size

	local tpk_pid_exists=1
	for tpk_iter in $(seq 20); do
		if [ ! -d "/proc/$MEMCG_PROCESS_PID" ] ||
			grep -q 'Z (zombie)' "/proc/$MEMCG_PROCESS_PID/status"; then
			tpk_pid_exists=0
			break
		fi

		tst_sleep 250ms
	done

	if [ $tpk_pid_exists -eq 0 ]; then
		wait $MEMCG_PROCESS_PID
		ret=$?
		if [ $ret -eq 1 ]; then
			tst_res TFAIL "process $MEMCG_PROCESS_PID is killed by error"
		elif [ $ret -eq 2 ]; then
			tst_res TPASS "Failed to lock memory"
		else
			tst_res TPASS "process $MEMCG_PROCESS_PID is killed"
		fi
	else
		stop_memcg_process
		tst_res TFAIL "process $MEMCG_PROCESS_PID is not killed"
	fi
}

# Test limit_in_bytes will be aligned to PAGESIZE
# $1 - user input value
# $2 - use mem+swap limitation
test_limit_in_bytes()
{
	local limit=$1
	local use_memsw=$2
	local elimit

	echo $limit > memory.limit_in_bytes
	if [ $use_memsw -eq 1 ]; then
		memcg_require_memsw
		echo $limit > memory.memsw.limit_in_bytes
		elimit=$(cat memory.memsw.limit_in_bytes)
	else
		elimit=$(cat memory.limit_in_bytes)
	fi

	# Kernels prior to 3.19 were rounding up,
	# but newer kernels are rounding down
	local limit_up=$(( PAGESIZE * (limit / PAGESIZE) ))
	local limit_down=$(( PAGESIZE * ((limit + PAGESIZE - 1) / PAGESIZE) ))
	if [ $limit_up -eq $elimit ] || [ $limit_down -eq $elimit ]; then
		tst_res TPASS "input=$limit, limit_in_bytes=$elimit"
	else
		tst_res TFAIL "input=$limit, limit_in_bytes=$elimit"
	fi
}

memcg_testfunc()
{
	ROD mkdir /dev/memcg/ltp_$$
	cd /dev/memcg/ltp_$$

	if type ${MEMCG_TESTFUNC}1 > /dev/null 2>&1; then
		${MEMCG_TESTFUNC}$1 $1 "$2"
	else
		${MEMCG_TESTFUNC} $1 "$2"
	fi

	cd $TST_TMPDIR
	ROD rmdir /dev/memcg/ltp_$$
}

memcg_no_testfunc()
{
	tst_brk TBROK "No testfunc specified, set MEMCG_TESTFUNC"
}
