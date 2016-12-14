#!/bin/bash
##############################################################################
#                                                                            #
# Copyright (c) International Business Machines  Corp., 2007                 #
#               Sivakumar Chinnaiah, Sivakumar.C@in.ibm.com                  #
# Copyright (c) Linux Test Project, 2016                                     #
#                                                                            #
# This program is free software: you can redistribute it and/or modify       #
# it under the terms of the GNU General Public License as published by       #
# the Free Software Foundation, either version 3 of the License, or          #
# (at your option) any later version.                                        #
#                                                                            #
# This program is distributed in the hope that it will be useful,            #
# but WITHOUT ANY WARRANTY; without even the implied warranty of             #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              #
# GNU General Public License for more details.                               #
#                                                                            #
# You should have received a copy of the GNU General Public License          #
# along with this program. If not, see <http://www.gnu.org/licenses/>.       #
#                                                                            #
##############################################################################
#                                                                            #
# Description:  Test Basic functionality of numactl command.                 #
#               Test #1: Verifies cpunodebind and membind                    #
#               Test #2: Verifies preferred node bind for memory allocation  #
#               Test #3: Verifies memory interleave on all nodes             #
#               Test #4: Verifies physcpubind                                #
#               Test #5: Verifies localalloc                                 #
#               Test #6: Verifies memory policies on shared memory           #
#               Test #7: Verifies numademo                                   #
#               Test #8: Verifies memhog                                     #
#               Test #9: Verifies numa_node_size api                         #
#               Test #10:Verifies Migratepages                               #
#                                                                            #
##############################################################################

TST_ID="numa01"
TST_CNT=10
TST_SETUP=setup
TST_TESTFUNC=test
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="numactl numastat awk"

. tst_test.sh

# Function:     extract_numastat
#
# Description:  - extract the value of given row, column from the numastat output.
#
# Input:        - $1 - row number.
#               - $2 - column number.
#
extract_numastat()
{
	RC=0

	# check whether numastat output is changed

	numastat > numalog

	RC=$(awk '
	{ if ( NR == '$2' ){
		print $1;
		}
	}
	' numalog)
	if [ $RC != $1 ]
	then
		tst_brk TBROK "numastat o/p seems to be changed, $1 expected to be in the row $2"
	fi

	RC=$(awk '
	{ if ( NR == '$2' ){
		print $'$3';
		}
	}
	' numalog)

	rm -f numalog

	echo $RC
}

# Function:     comparelog
#
# Description:  - return the difference of input arguments if they are in
#                 increasing order.
#
# Input:        - $1 - original value.
#               - $2 - changed value.
#
# Return:       - difference of arguments on success.
comparelog()
{
	if [ $2 -gt $1 ]; then
		RC=$(($2-$1))
	else
		RC=0
	fi

	echo $RC
}

# Function: setup
#
# Description:  - Check if command required for this test exits.
#               - Initialize global variables.
#
setup()
{
	export MB=$((1024*1024))
	export PAGE_SIZE=$(getconf PAGE_SIZE)

	# row definitions, pls see at the top of this file
	numa_hit=2
	numa_miss=3
	numa_foreign=4
	interleave_hit=5
	local_node=6
	other_node=7

	# arguments to memory exercise program support_numa.c
	ALLOC_1MB=1
	PAUSE=2

	total_nodes=0       # total no. of numa nodes
	# all availiable nodes id list
	nodes_list=$(numactl --show | grep nodebind | cut -d ':' -f 2)
	for node in $nodes_list; do
		total_nodes=$((total_nodes+1))
	done
	tst_res TINFO "The system contains $total_nodes nodes: $nodes_list"
	if [ $total_nodes -le 1 ]; then
		tst_res TCONF "your machine does not support numa policy
		or your machine is not a NUMA machine"
		exit 0
	fi
}

# Function:     test1
#
# Description:  - Verification of local node and memory affinity
#
test1()
{
	RC=0                # Return value from commands.
	Prev_value=0        # extracted from the numastat o/p
	Curr_value=0        # Current value extracted from numastat o/p
	Exp_incr=0          # 1 MB/ PAGESIZE
	col=0

	# Increase in numastat o/p is interms of pages
	Exp_incr=$((MB/PAGE_SIZE))

	COUNTER=1
	for node in $nodes_list; do
		col=$((COUNTER+1))		# Node number in numastat o/p
		Prev_value=$(extract_numastat local_node $local_node $col)
		numactl --cpunodebind=$node --membind=$node support_numa $ALLOC_1MB
		Curr_value=$(extract_numastat local_node $local_node $col)

		RC=$(comparelog $Prev_value $Curr_value)
		if [ $RC -lt $Exp_incr ]; then
			tst_res TFAIL \
				"NUMA hit and localnode increase in node$node is less than expected"
			return
		fi
		COUNTER=$((COUNTER+1))
	done

	tst_res TPASS "NUMA local node and memory affinity -TEST01 PASSED !!"
}

# Function:     test2
#
# Description:  - Verification of memory allocated from preferred node
#
test2()
{
	RC=0                # Return value from commands.
	Prev_value=0        # extracted from the numastat o/p
	Curr_value=0        # Current value extracted from numastat o/p
	Exp_incr=0          # 1 MB/ PAGESIZE
	col=0

	# Increase in numastat o/p is interms of pages
	Exp_incr=$((MB/PAGE_SIZE))

	COUNTER=1
	for node in $nodes_list; do

		if [ $COUNTER -eq $total_nodes ]   #wrap up for last node
		then
			Preferred_node=$(echo $nodes_list | cut -d ' ' -f 1)
			col=2			   #column represents node0 in numastat o/p
		else
			# always next node is preferred node
			Preferred_node=$(echo $nodes_list | cut -d ' ' -f $((COUNTER+1)))
			col=$((COUNTER+2))         #Preferred Node number in numastat o/p
		fi

		Prev_value=$(extract_numastat other_node $other_node $col)
		numactl --cpunodebind=$node --preferred=$Preferred_node support_numa $ALLOC_1MB

		Curr_value=$(extract_numastat other_node $other_node $col)
		RC=$(comparelog $Prev_value $Curr_value)
		if [ $RC -lt $Exp_incr ]; then
			tst_res TFAIL \
				"NUMA hit and othernode increase in node$node is less than expected"
			return
		fi
		COUNTER=$((COUNTER+1))
	done

	tst_res TPASS "NUMA preferred node policy -TEST02 PASSED !!"
}

# Function:     test3
#
# Description:  - Verification of memory interleaved on all nodes
#
test3()
{
	RC=0                # Return value from commands.
	Prev_value=0        # extracted from the numastat o/p
	Pstr_value=""       # string contains previous value of all nodes
	Curr_value=0        # Current value extracted from numastat o/p
	Exp_incr=0          # 1 MB/ (PAGESIZE*num_of_nodes)
	col=0

	# Increase in numastat o/p is interms of pages
	Exp_incr=$((MB/PAGE_SIZE))
	# Pages will be allocated using round robin on nodes.
	Exp_incr=$((Exp_incr/total_nodes))

	# Check whether the pages are equally distributed among available nodes
	COUNTER=1
	for node in $nodes_list; do
		col=$((COUNTER+1))              #Node number in numastat o/p
		Prev_value=$(extract_numastat interleave_hit $interleave_hit $col)
		Pstr_value+="$Prev_value "
		COUNTER=$((COUNTER+1))
	done

	numactl --interleave=all support_numa $ALLOC_1MB

	COUNTER=1
	for node in $nodes_list; do
		col=$((COUNTER+1))             #Node number in numastat o/p
		Curr_value=$(extract_numastat interleave_hit $interleave_hit $col)
		RC=$(comparelog $(echo $Pstr_value | cut -d ' ' -f $COUNTER) $Curr_value)
		if [ $RC -lt $Exp_incr ]; then
			tst_res TFAIL \
				"NUMA interleave hit in node$node is less than expected"
			return
		fi
		COUNTER=$((COUNTER+1))
	done
	tst_res TPASS "NUMA interleave policy -TEST03 PASSED !!"
}

# Function:     test4
#
# Description:  - Verification of physical cpu bind
#
test4()
{
	no_of_cpus=0	#no. of cpu's exist
	run_on_cpu=0
	running_on_cpu=0

	no_of_cpus=$(tst_ncpus)
	# not sure whether cpu's can't be in odd number
	run_on_cpu=$(($((no_of_cpus+1))/2))
	numactl --physcpubind=$run_on_cpu support_numa $PAUSE & #just waits for sigint
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
		return
	fi
	RC=0
	kill -9 $pid || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brk TBROK "Kill on process $pid fails"
	fi

	tst_res TPASS "NUMA phycpubind policy -TEST04 PASSED !!"
}

# Function:     test5
#
# Description:  - Verification of local node allocation
#
test5()
{
	RC=0                # Return value from commands.
	Prev_value=0        # extracted from the numastat o/p
	Curr_value=0        # Current value extracted from numastat o/p
	Exp_incr=0          # 1 MB/ PAGESIZE
	col=0

	# Increase in numastat o/p is interms of pages
	Exp_incr=$((MB/PAGE_SIZE))

	COUNTER=1
	for node in $nodes_list; do
		col=$((COUNTER+1))               #Node number in numastat o/p
		Prev_value=$(extract_numastat local_node $local_node $col)
		numactl --cpunodebind=$node --localalloc support_numa $ALLOC_1MB
		Curr_value=$(extract_numastat local_node $local_node $col)
		RC=$(comparelog $Prev_value $Curr_value)
		if [ $RC -lt $Exp_incr ]; then
			tst_res TFAIL \
				"NUMA hit and localnode increase in node$node is less than expected"
			return
		fi
		COUNTER=$((COUNTER+1))
	done

	tst_res TPASS "NUMA local node allocation -TEST05 PASSED !!"
}

# Function:     test6
#
# Description:  - Verification of shared memory interleaved on all nodes
#
test6()
{
	RC=0                # Return value from commands.
	Prev_value=0        # extracted from the numastat o/p
	Pstr_value=""       # string contains previous value of all nodes
	Curr_value=0        # Current value extracted from numastat o/p
	Exp_incr=0          # 1 MB/ (PAGESIZE*num_of_nodes)
	col=0

	# Increase in numastat o/p is interms of pages
	Exp_incr=$((MB/PAGE_SIZE))
	# Pages will be allocated using round robin on nodes.
	Exp_incr=$((Exp_incr/total_nodes))

	# Check whether the pages are equally distributed among available nodes
	COUNTER=1
	for node in $nodes_list; do
		col=$((COUNTER+1))              #Node number in numastat o/p
		Prev_value=$(extract_numastat numa_hit $numa_hit $col)
		Pstr_value+="$Prev_value "
		COUNTER=$((COUNTER+1))
	done

	numactl --length=1M --file /dev/shm/ltp_numa_shm --interleave=all --touch

	COUNTER=1
	for node in $nodes_list; do
		col=$((COUNTER+1))              #Node number in numastat o/p
		Curr_value=$(extract_numastat numa_hit $numa_hit $col)
		RC=$(comparelog $(echo $Pstr_value | cut -d ' ' -f $COUNTER) $Curr_value)
		if [ $RC -lt $Exp_incr ]; then
			tst_res TFAIL \
				"NUMA numa_hit for shm file ltp_numa_shm in node$node is less than expected"
			return
		fi
		COUNTER=$((COUNTER+1))
	done

	tst_res TPASS "NUMA interleave policy on shared memory -TEST06 PASSED !!"
	RC=0
	rm -r /dev/shm/ltp_numa_shm || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL "Failed removing shared memory file ltp_numa_shm"
	fi
}

# Function:     test7
#
# Description:  - Verification of numademo
#
test7()
{
	RC=0                # Return value from commands.
	Prev_value=0        # extracted from the numastat o/p
	Pstr_value=""       # string contains previous value of all nodes
	Curr_value=0        # Current value extracted from numastat o/p
	Exp_incr=0          # 1 MB/ (PAGESIZE*num_of_nodes)
	col=0
	msize=1000
	KB=1024
	# Increase in numastat o/p is interms of pages
	Exp_incr=$(($((KB * msize))/PAGE_SIZE))
	# Pages will be allocated using round robin on nodes.
	Exp_incr=$((Exp_incr/total_nodes))

	# Check whether the pages are equally distributed among available nodes
	COUNTER=1
	for node in $nodes_list; do
		col=$((COUNTER+1))              #Node number in numastat o/p
		Prev_value=$(extract_numastat interleave_hit $interleave_hit $col)
		Pstr_value+="$Prev_value "
		COUNTER=$((COUNTER+1))
	done

	numademo -c ${msize}k > gdemolog

	COUNTER=1
	x=0
	for node in $nodes_list; do
		col=$((COUNTER+1))              #Node number in numastat o/p
		Curr_value=$(extract_numastat interleave_hit $interleave_hit $col)
		RC=$(comparelog $(echo $Pstr_value | cut -d ' ' -f $COUNTER) $Curr_value)
		if [ $RC -le $Exp_incr ]; then
			x=1
			break;
		fi
		COUNTER=$((COUNTER+1))
	done
	if [ $x -eq 0 ]; then
		tst_res TPASS "NUMADEMO policies  -TEST07 PASSED !!"
	else
		tst_res TFAIL "NUMA interleave hit is less than expected"
	fi
}

# Function:     test8
#
# Description:  - Verification of memhog with interleave policy
#
test8()
{
	RC=0                # Return value from commands.
	Prev_value=0        # extracted from the numastat o/p
	Pstr_value=""       # string contains previous value of all nodes
	Curr_value=0        # Current value extracted from numastat o/p
	Exp_incr=0          # 1 MB/ (PAGESIZE*num_of_nodes)
	col=0

	# Increase in numastat o/p is interms of pages
	Exp_incr=$((MB/$PAGE_SIZE))
	# Pages will be allocated using round robin on nodes.
	Exp_incr=$((Exp_incr/total_nodes))

	# Check whether the pages are equally distributed among available nodes
	COUNTER=1
	for node in $nodes_list; do
		col=$((COUNTER+1))              #Node number in numastat o/p
		Prev_value=$(extract_numastat interleave_hit $interleave_hit $col)
		Pstr_value+="$Prev_value "
		COUNTER=$((COUNTER+1))
	done
	numactl --interleave=all memhog 1MB

	COUNTER=1
	for node in $nodes_list; do
		col=$((COUNTER+1))              #Node number in numastat o/p
		Curr_value=$(extract_numastat interleave_hit $interleave_hit $col)
		RC=$(comparelog $(echo $Pstr_value | cut -d ' ' -f $COUNTER) $Curr_value)
		if [ $RC -lt $Exp_incr ]; then
			tst_res TFAIL \
				"NUMA interleave hit in node$node is less than expected"
			return
		fi
		COUNTER=$((COUNTER+1))
	done
	tst_res TPASS "NUMA MEMHOG policy -TEST08 PASSED !!"
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
test9()
{
	RC=0                # Return value from commands.

	numactl --hardware > gavail_nodes
	RC=$(awk '{ if ( NR == 1 ) {print $1;} }' gavail_nodes)
	if [ $RC = "available:" ]; then
		RC=$(awk '{ if ( NR == 1 ) {print $3;} }' gavail_nodes)
		if [ $RC = "nodes" ]; then
			RC=$(awk '{ if ( NR == 1 ) {print $2;} }' gavail_nodes)
			tst_res TPASS "NUMA policy on lib NUMA_NODE_SIZE API -TEST09 PASSED !!"
		else
			tst_res TFAIL "Failed with numa policy"
		fi
	else
		tst_res TFAIL "Failed with numa policy"
	fi
}

# Function:     test10
#
# Description:  - Verification of migratepages
#
test10()
{
	RC=0
	Prev_value=0
	Curr_value=0

	COUNTER=1
	for node in $nodes_list; do

		if [ $COUNTER -eq $total_nodes ]; then
			Preferred_node=`echo $nodes_list | cut -d ' ' -f 1`
			col=2
		else
			Preferred_node=`echo $nodes_list | cut -d ' ' -f $[$COUNTER+1]`
			col=$((COUNTER+2))
		fi

		Prev_value=$(extract_numastat other_node $other_node $col)
		numactl --preferred=$node support_numa $PAUSE &
		pid=$!
		migratepages $pid $node $Preferred_node
		Curr_value=$(extract_numastat other_node $other_node $col)
		kill -9 $pid
		if [ $Curr_value -lt $Prev_value ]; then
			tst_res TFAIL \
				"NUMA migratepages is not working fine"
			return
		fi
		COUNTER=$((COUNTER+1))
	done

	tst_res TPASS "NUMA MIGRATEPAGES policy -TEST10 PASSED !!"
}

tst_run
