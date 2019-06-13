#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2007
# Copyright (c) Linux Test Project, 2016
# Author: Sivakumar Chinnaiah <Sivakumar.C@in.ibm.com>
#
# Test Basic functionality of numactl command.
# Test #1: Verifies cpunodebind and membind
# Test #2: Verifies preferred node bind for memory allocation
# Test #3: Verifies memory interleave on all nodes
# Test #4: Verifies physcpubind
# Test #5: Verifies localalloc
# Test #6: Verifies memhog
# Test #7: Verifies numa_node_size api
# Test #8:Verifies Migratepages
# Test #9:Verifies hugepage alloacted on specified node
# Test #10:Verifies THP memory allocated on preferred node

TST_CNT=10
TST_SETUP=setup
TST_TESTFUNC=test
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="awk bc numactl numastat"

. tst_test.sh

# Extracts the value of given numa node from the `numastat -p` output.
# $1 - Pid number.
# $2 - Node number.
extract_numastat_p()
{
	local pid=$1
	local node=$(($2 + 2))

	echo $(numastat -p $pid |awk '/^Total/ {print $'$node'}')
}

check_for_support_numa()
{
	local pid=$1

	local state=$(awk '{print $3}' /proc/$pid/stat)

	if [ $state = 'T' ]; then
		return 0
	fi

	return 1
}

setup()
{
	export MB=$((1024*1024))
	export PAGE_SIZE=$(tst_getconf PAGESIZE)
	export HPAGE_SIZE=$(awk '/Hugepagesize:/ {print $2}' /proc/meminfo)

	total_nodes=0

	nodes_list=$(numactl --show | grep nodebind | cut -d ':' -f 2)
	for node in $nodes_list; do
		total_nodes=$((total_nodes+1))
	done

	tst_res TINFO "The system contains $total_nodes nodes: $nodes_list"
	if [ $total_nodes -le 1 ]; then
		tst_brk TCONF "your machine does not support numa policy
		or your machine is not a NUMA machine"
	fi
}

# Verification of memory allocated on a node
test1()
{
	Mem_curr=0

	for node in $nodes_list; do
		numactl --cpunodebind=$node --membind=$node support_numa alloc_1MB &
		pid=$!

		TST_RETRY_FUNC "check_for_support_numa $pid" 0

		Mem_curr=$(echo "$(extract_numastat_p $pid $node) * $MB" |bc)
		if [ $(echo "$Mem_curr < $MB" | bc) -eq 1 ]; then
			tst_res TFAIL \
				"NUMA memory allocated in node$node is less than expected"
			kill -CONT $pid >/dev/null 2>&1
			return
		fi

		kill -CONT $pid >/dev/null 2>&1
	done

	tst_res TPASS "NUMA local node and memory affinity"
}

# Verification of memory allocated on preferred node
test2()
{
	Mem_curr=0

	COUNTER=1
	for node in $nodes_list; do

		if [ $COUNTER -eq $total_nodes ]; then   #wrap up for last node
			Preferred_node=$(echo $nodes_list | cut -d ' ' -f 1)
		else
			# always next node is preferred node
			Preferred_node=$(echo $nodes_list | cut -d ' ' -f $((COUNTER+1)))
		fi

		numactl --cpunodebind=$node --preferred=$Preferred_node support_numa alloc_1MB &
		pid=$!

		TST_RETRY_FUNC "check_for_support_numa $pid" 0

		Mem_curr=$(echo "$(extract_numastat_p $pid $Preferred_node) * $MB" |bc)
		if [ $(echo "$Mem_curr < $MB" |bc ) -eq 1 ]; then
			tst_res TFAIL \
				"NUMA memory allocated in node$Preferred_node is less than expected"
			kill -CONT $pid >/dev/null 2>&1
			return
		fi

		COUNTER=$((COUNTER+1))
		kill -CONT $pid >/dev/null 2>&1
	done

	tst_res TPASS "NUMA preferred node policy"
}

# Verification of memory interleaved on all nodes
test3()
{
	Mem_curr=0
	# Memory will be allocated using round robin on nodes.
	Exp_incr=$(echo "$MB / $total_nodes" |bc)

	numactl --interleave=all support_numa alloc_1MB &
	pid=$!

	TST_RETRY_FUNC "check_for_support_numa $pid" 0

	for node in $nodes_list; do
		Mem_curr=$(echo "$(extract_numastat_p $pid $node) * $MB" |bc)

		if [ $(echo "$Mem_curr < $Exp_incr" |bc ) -eq 1 ]; then
			tst_res TFAIL \
				"NUMA interleave memory allocated in node$node is less than expected"
			kill -CONT $pid >/dev/null 2>&1
			return
		fi
	done

	kill -CONT $pid >/dev/null 2>&1
	tst_res TPASS "NUMA interleave policy"
}

# Verification of physical cpu bind
test4()
{
	no_of_cpus=0	#no. of cpu's exist
	run_on_cpu=0
	running_on_cpu=0

	no_of_cpus=$(tst_ncpus)
	# not sure whether cpu's can't be in odd number
	run_on_cpu=$(($((no_of_cpus+1))/2))
	numactl --physcpubind=$run_on_cpu support_numa pause & #just waits for sigint
	pid=$!
	var=`awk '{ print $2 }' /proc/$pid/stat`
	while [ $var = '(numactl)' ]; do
		var=`awk '{ print $2 }' /proc/$pid/stat`
		tst_sleep 100ms
	done
	# Warning !! 39 represents cpu number, on which process pid is currently running and
	# this may change if Some more fields are added in the middle, may be in future
	running_on_cpu=$(awk '{ print $39; }' /proc/$pid/stat)
	if [ $running_on_cpu -ne $run_on_cpu ]; then
		tst_res TFAIL \
			"Process running on cpu$running_on_cpu but expected to run on cpu$run_on_cpu"
		ROD kill -INT $pid
		return
	fi

	ROD kill -INT $pid

	tst_res TPASS "NUMA phycpubind policy"
}

# Verification of local node allocation
test5()
{
	Mem_curr=0

	for node in $nodes_list; do
		numactl --cpunodebind=$node --localalloc support_numa alloc_1MB &
		pid=$!

		TST_RETRY_FUNC "check_for_support_numa $pid" 0

		Mem_curr=$(echo "$(extract_numastat_p $pid $node) * $MB" |bc)
		if [ $(echo "$Mem_curr < $MB" |bc ) -eq 1 ]; then
			tst_res TFAIL \
				"NUMA localnode memory allocated in node$node is less than expected"
			kill -CONT $pid >/dev/null 2>&1
			return
		fi

		kill -CONT $pid >/dev/null 2>&1
	done

	tst_res TPASS "NUMA local node allocation"
}

check_ltp_numa_test8_log()
{
	grep -m1 -q '.' ltp_numa_test8.log
}

# Verification of memhog with interleave policy
test6()
{
	Mem_curr=0
	# Memory will be allocated using round robin on nodes.
	Exp_incr=$(echo "$MB / $total_nodes" |bc)

	numactl --interleave=all memhog -r1000000 1MB >ltp_numa_test8.log 2>&1 &
	pid=$!

	TST_RETRY_FUNC "check_ltp_numa_test8_log" 0

	for node in $nodes_list; do
		Mem_curr=$(echo "$(extract_numastat_p $pid $node) * $MB" |bc)

		if [ $(echo "$Mem_curr < $Exp_incr" |bc ) -eq 1 ]; then
			tst_res TFAIL \
				"NUMA interleave memhog in node$node is less than expected"
			kill -KILL $pid >/dev/null 2>&1
			return
		fi
	done

	kill -KILL $pid >/dev/null 2>&1
	tst_res TPASS "NUMA MEMHOG policy"
}

# Function:     hardware cheking with numa_node_size api
#
# Description:  - Returns the size of available nodes if success.
#
# Input:        - o/p of numactl --hardware command which is expected in the format
#                 shown below
#               available: 2 nodes (0-1)
#               node 0 size: 7808 MB
#               node 0 free: 7457 MB
#               node 1 size: 5807 MB
#               node 1 free: 5731 MB
#               node distances:
#               node   0   1
#                 0:  10  20
#                 1:  20  10
#
test7()
{
	RC=0

	numactl --hardware > gavail_nodes
	RC=$(awk '{ if ( NR == 1 ) {print $1;} }' gavail_nodes)
	if [ $RC = "available:" ]; then
		RC=$(awk '{ if ( NR == 1 ) {print $3;} }' gavail_nodes)
		if [ $RC = "nodes" ]; then
			RC=$(awk '{ if ( NR == 1 ) {print $2;} }' gavail_nodes)
			tst_res TPASS "NUMA policy on lib NUMA_NODE_SIZE API"
		else
			tst_res TFAIL "Failed with numa policy"
		fi
	else
		tst_res TFAIL "Failed with numa policy"
	fi
}

# Verification of migratepages
test8()
{
	Mem_curr=0
	COUNTER=1

	for node in $nodes_list; do

		if [ $COUNTER -eq $total_nodes ]; then
			Preferred_node=$(echo $nodes_list | cut -d ' ' -f 1)
		else
			Preferred_node=$(echo $nodes_list | cut -d ' ' -f $((COUNTER+1)))
		fi

		numactl --preferred=$node support_numa alloc_1MB &
		pid=$!

		TST_RETRY_FUNC "check_for_support_numa $pid" 0

		migratepages $pid $node $Preferred_node

		Mem_curr=$(echo "$(extract_numastat_p $pid $Preferred_node) * $MB" |bc)
		if [ $(echo "$Mem_curr < $MB" |bc ) -eq 1 ]; then
			tst_res TFAIL \
				"NUMA migratepages is not working fine"
			kill -CONT $pid >/dev/null 2>&1
			return
		fi

		COUNTER=$((COUNTER+1))
		kill -CONT $pid >/dev/null 2>&1
	done

	tst_res TPASS "NUMA MIGRATEPAGES policy"
}

# Verification of hugepage memory allocated on a node
test9()
{
	Mem_huge=0
	Sys_node=/sys/devices/system/node

	if [ ! -d "/sys/kernel/mm/hugepages/" ]; then
		tst_res TCONF "hugepage is not supported"
		return
	fi

	for node in $nodes_list; do
		Ori_hpgs=$(cat ${Sys_node}/node${node}/hugepages/hugepages-${HPAGE_SIZE}kB/nr_hugepages)
		New_hpgs=$((Ori_hpgs + 1))
		echo $New_hpgs >${Sys_node}/node${node}/hugepages/hugepages-${HPAGE_SIZE}kB/nr_hugepages

		Chk_hpgs=$(cat ${Sys_node}/node${node}/hugepages/hugepages-${HPAGE_SIZE}kB/nr_hugepages)
		if [ "$Chk_hpgs" -ne "$New_hpgs" ]; then
			tst_res TCONF "hugepage is not enough to test"
			return
		fi

		numactl --cpunodebind=$node --membind=$node support_numa alloc_1huge_page &
		pid=$!
		TST_RETRY_FUNC "check_for_support_numa $pid" 0

		Mem_huge=$(echo $(numastat -p $pid |awk '/^Huge/ {print $'$((node+2))'}'))
		Mem_huge=$((${Mem_huge%.*} * 1024))

		if [ "$Mem_huge" -lt "$HPAGE_SIZE" ]; then
			tst_res TFAIL \
				"NUMA memory allocated in node$node is less than expected"
			kill -CONT $pid >/dev/null 2>&1
			echo $Ori_hpgs >${Sys_node}/node${node}/hugepages/hugepages-${HPAGE_SIZE}kB/nr_hugepages
			return
		fi

		kill -CONT $pid >/dev/null 2>&1
		echo $Ori_hpgs >${Sys_node}/node${node}/hugepages/hugepages-${HPAGE_SIZE}kB/nr_hugepages
	done

	tst_res TPASS "NUMA local node hugepage memory allocated"
}

# Verification of THP memory allocated on preferred node
test10()
{
	Mem_curr=0

	if ! grep -q '\[always\]' /sys/kernel/mm/transparent_hugepage/enabled; then
		tst_res TCONF "THP is not supported/enabled"
		return
	fi

	COUNTER=1
	for node in $nodes_list; do

		if [ $COUNTER -eq $total_nodes ]; then   #wrap up for last node
			Preferred_node=$(echo $nodes_list | cut -d ' ' -f 1)
		else
			# always next node is preferred node
			Preferred_node=$(echo $nodes_list | cut -d ' ' -f $((COUNTER+1)))
		fi

		numactl --cpunodebind=$node --preferred=$Preferred_node support_numa alloc_2HPSZ_THP &
		pid=$!

		TST_RETRY_FUNC "check_for_support_numa $pid" 0

		Mem_curr=$(echo "$(extract_numastat_p $pid $Preferred_node) * 1024" |bc)
		if [ $(echo "$Mem_curr < $HPAGE_SIZE * 2" |bc ) -eq 1 ]; then
			tst_res TFAIL \
				"NUMA memory allocated in node$Preferred_node is less than expected"
			kill -CONT $pid >/dev/null 2>&1
			return
		fi

		COUNTER=$((COUNTER+1))
		kill -CONT $pid >/dev/null 2>&1
	done

	tst_res TPASS "NUMA preferred node policy verified with THP enabled"
}

tst_run
