#!/bin/sh

################################################################################
#                                                                              #
# Copyright (c) 2009 FUJITSU LIMITED                                           #
#                                                                              #
# This program is free software;  you can redistribute it and#or modify        #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful, but          #
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY   #
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License     #
# for more details.                                                            #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program;  if not, write to the Free Software                 #
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA      #
#                                                                              #
# Author: Miao Xie <miaox@cn.fujitsu.com>                                      #
#                                                                              #
################################################################################

export TCID="cpuset_memory_spread"
export TST_TOTAL=6
export TST_COUNT=1

. cpuset_funcs.sh

check

exit_status=0
nr_cpus=$NR_CPUS
nr_mems=$N_NODES

# In general, the cache hog will use more than 10000 kb slab space on the nodes
# on which it is running. The other nodes' slab space has littler change.(less
# than 1000 kb).
upperlimit=10000

# set lowerlimit according to pagesize
# pagesize(bytes)  | lowerlimit(kb)
# ------------------------------------
#  4096            | 2048
#  16384           | 8192

PAGE_SIZE=`tst_getconf PAGESIZE`
lowerlimit=$((PAGE_SIZE * 512 / 1024))

cpus_all="$(seq -s, 0 $((nr_cpus-1)))"
mems_all="$(seq -s, 0 $((nr_mems-1)))"

nodedir="/sys/devices/system/node"

FIFO="./myfifo"

# memsinfo is an array implementation of the form of a multi-line string
# _0: value0
# _1: value1
# _2: value2
#
memsinfo=""

# set value to memsinfo ($1 - index, $2 - value)
set_memsinfo_val()
{
	local nl='
'
	# clearing existent value (if present)
	memsinfo=`echo "$memsinfo" | sed -r "/^\_$1\: /d"`

	if [ -z "$memsinfo" ]; then
		memsinfo="_$1: $2"
	else
		memsinfo="$memsinfo${nl}_$1: $2"
	fi
}

# get value from memsinfo ($1 - index)
get_memsinfo_val()
{
	local value=
	value=`echo "$memsinfo" | grep -e "^\_$1\: "`
	value=`echo "$value" | sed -r "s/^.*\: (.*)$/\1/"`
	echo "$value"
}


init_memsinfo_array()
{
	local i=

	for i in `seq 0 $((nr_mems-1))`
	do
		set_memsinfo_val $i 0
	done
}

# get_meminfo <nodeid> <item>
get_meminfo()
{
	local nodeid="$1"
	local nodepath="$nodedir/node$nodeid"
	local nodememinfo="$nodepath/meminfo"
	local item="$2"
	local info=`cat $nodememinfo | grep $item | awk '{print $4}'`
	set_memsinfo_val $nodeid $info
}

# freemem_check
# We need enough memory space on every node to run this test, so we must check
# whether every node has enough free memory or not.
# return value: 1 - Some node doesn't have enough free memory
#               0 - Every node has enough free memory, We can do this test
freemem_check()
{
	local i=

	for i in `seq 0 $((nr_mems-1))`
	do
		get_meminfo $i "MemFree"
	done

	for i in `seq 0 $((nr_mems-1))`
	do
		# I think we need 100MB free memory to run test
		if [ $(get_memsinfo_val $i) -lt 100000 ]; then
			return 1
		fi
	done
}

# get_memsinfo
get_memsinfo()
{
	local i=

	for i in `seq 0 $((nr_mems-1))`
	do
		get_meminfo $i "FilePages"
	done
}

# account_meminfo <nodeId>
account_meminfo()
{
	local nodeId="$1"
	local tmp="$(get_memsinfo_val $nodeId)"
	get_meminfo $@ "FilePages"
	set_memsinfo_val $nodeId $(($(get_memsinfo_val $nodeId)-$tmp))
}

# account_memsinfo
account_memsinfo()
{
	local i=

	for i in `seq 0 $((nr_mems-1))`
	do
		account_meminfo $i
	done
}


# result_check <nodelist>
# return 0: success
#	 1: fail
result_check()
{
	local nodelist="`echo $1 | sed -e 's/,/ /g'`"
	local i=

	for i in $nodelist
	do
		if [ $(get_memsinfo_val $i) -le $upperlimit ]; then
			return 1
		fi
	done

	local allnodelist="`echo $mems_all | sed -e 's/,/ /g'`"
	allnodelist=" "$allnodelist" "
	nodelist=" "$nodelist" "

	local othernodelist="$allnodelist"
	for i in $nodelist
	do
		othernodelist=`echo "$othernodelist" | sed -e "s/ $i / /g"`
	done

	for i in $othernodelist
	do
		if [ $(get_memsinfo_val $i) -gt $lowerlimit ]; then
			return 1
		fi
	done
}

# general_memory_spread_test <cpusetpath> <is_spread> <cpu_list> <node_list> \
# <expect_nodes> <test_pid>
# expect_nodes: we expect to use the slab or cache on which node
general_memory_spread_test()
{
	local cpusetpath="$CPUSET/1"
	local is_spread="$1"
	local cpu_list="$2"
	local node_list="$3"
	local expect_nodes="$4"
	local test_pid="$5"

	cpuset_set "$cpusetpath" "$cpu_list" "$node_list" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	/bin/echo "$is_spread" > "$cpusetpath/cpuset.memory_spread_page" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set spread value failed."
		return 1
	fi

	/bin/echo "$test_pid" > "$cpusetpath/tasks" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "attach task failed."
		return 1
	fi

	# we'd better drop the caches before we test page cache.
	sync
	/bin/echo 3 > /proc/sys/vm/drop_caches 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "drop caches failed."
		return 1
	fi

	get_memsinfo
	/bin/kill -s SIGUSR1 $test_pid
	read exit_num < $FIFO
	if [ $exit_num -eq 0 ]; then
		tst_resm TFAIL "hot mem task failed."
		return 1
	fi

	account_memsinfo
	result_check $expect_nodes
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "hog the memory on the unexpected node(FilePages_For_Nodes(KB): ${memsinfo}, Expect Nodes: $expect_nodes)."
		return 1
	fi
}

base_test()
{
	local pid=
	local result_num=

	setup
	if [ $? -ne 0 ]; then
		exit_status=1
	else
		cpuset_mem_hog &
		pid=$!
		general_memory_spread_test "$@" "$pid"
		result_num=$?
		if [ $result_num -ne 0 ]; then
			exit_status=1
		fi

		/bin/kill -s SIGUSR2 $pid
		wait $pid

		cleanup
		if [ $? -ne 0 ]; then
			exit_status=1
		elif [ $result_num -eq 0 ]; then
			tst_resm TPASS "Cpuset memory spread page test succeeded."
		fi
	fi
	TST_COUNT=$(($TST_COUNT + 1))
}

# test general spread page cache in a cpuset
test_spread_page1()
{
	while read spread cpus nodes exp_nodes
	do
		base_test "$spread" "$cpus" "$nodes" "$exp_nodes"
	done <<- EOF
		0	0	0	0
		1	0	0	0
		0	0	1	1
		1	0	1	1
		0	0	0,1	0
		1	0	0,1	0,1
	EOF
	# while read spread cpus nodes exp_nodes
}

test_spread_page2()
{
	local pid=
	local result_num=

	setup
	if [ $? -ne 0 ]; then
		exit_status=1
	else
		cpuset_mem_hog &
		pid=$!
		general_memory_spread_test "1" "$cpus_all" "0" "0" "$pid"
		result_num=$?
		if [ $result_num -ne 0 ]; then
			exit_status=1
		else
			general_memory_spread_test "1" "$cpus_all" "1" "1" "$pid"
			result_num=$?
			if [ $result_num -ne 0 ]; then
				exit_status=1
			fi
		fi

		/bin/kill -s SIGUSR2 $pid
		wait $pid

		cleanup
		if [ $? -ne 0 ]; then
			exit_status=1
		elif [ $result_num -eq 0 ]; then
			tst_resm TPASS "Cpuset memory spread page test succeeded."
		fi
	fi
}

init_memsinfo_array
freemem_check
if [ $? -ne 0 ]; then
	tst_brkm TCONF "Some node doesn't has enough free memory(100MB) to do test(MemFree_For_Nodes(KB): ${memsinfo[*]})."
fi

dd if=/dev/zero of=./DATAFILE bs=1M count=100
if [ $? -ne 0 ]; then
	tst_brkm TFAIL "Creating DATAFILE failed."
fi

rm -f $FIFO
mkfifo $FIFO
if [ $? -ne 0 ]; then
	rm -f DATAFILE
	tst_brkm TFAIL "failed to mkfifo $FIFO"
fi

test_spread_page1
test_spread_page2

rm -f DATAFILE $FIFO

exit $exit_status
