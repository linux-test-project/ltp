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

export TCID="cpuset_memory"
export TST_TOTAL=17
export TST_COUNT=1

. cpuset_funcs.sh

check

exit_status=0

nr_cpus=$NR_CPUS
nr_mems=$N_NODES

cpus_all="$(seq -s, 0 $((nr_cpus-1)))"
mems_all="$(seq -s, 0 $((nr_mems-1)))"

cpu_of_node0=0

HUGEPAGESIZE=$(awk '/Hugepagesize/{ print $2 }' /proc/meminfo)
HUGEPAGESIZE=$((${HUGEPAGESIZE:-0} * 1024))

MEMORY_RESULT="$CPUSET_TMP/memory_result"

# simple_getresult
# $1 - cpuset_memory_test's pid
# $2 - move cpuset_memory_test's pid to this cpuset
simple_getresult()
{
	sleep 1
	echo $1 > "$2/tasks"
	/bin/kill -s SIGUSR1 $1
	sleep 1
	/bin/kill -s SIGUSR1 $1
	sleep 1
	/bin/kill -s SIGINT $1
	wait $1
	read node < "$MEMORY_RESULT"
}

test1()
{
	cpuset_set "$CPUSET/0" "$cpu_of_node0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	cpuset_memory_test --mmap-anon >"$MEMORY_RESULT" &
	simple_getresult $! "$CPUSET/0"
	if [ "$node" != "0" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node(Expect: Node#0)."
		return 1
	fi
}

test2()
{
	cpuset_set "$CPUSET/0" "$cpu_of_node0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	cpuset_memory_test --mmap-file >"$MEMORY_RESULT" &
	simple_getresult $! "$CPUSET/0"
	if [ "$node" != "0" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node(Expect: Node#0)."
		return 1
	fi
}

test3()
{
	cpuset_set "$CPUSET/0" "$cpu_of_node0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	cpuset_memory_test --shm >"$MEMORY_RESULT" &
	simple_getresult $! "$CPUSET/0"
	if [ "$node" != "0" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node(Expect: Node#0)."
		return 1
	fi
}

test4()
{
	cpuset_set "$CPUSET/0" "$cpu_of_node0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	cpuset_memory_test --mmap-lock1 >"$MEMORY_RESULT" &
	simple_getresult $! "$CPUSET/0"
	if [ "$node" != "0" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node(Expect: Node#0)."
		return 1
	fi
}

test5()
{
	cpuset_set "$CPUSET/0" "$cpu_of_node0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	cpuset_memory_test --mmap-lock2 >"$MEMORY_RESULT" &
	simple_getresult $! "$CPUSET/0"
	if [ "$node" != "0" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node(Expect: Node#0)."
		return 1
	fi
}

# check whether the system supports hugetlbfs or not
# return 0 - don't support hugetlbfs
#        1 - support hugetlbfs
check_hugetlbfs()
{
	local fssupport=$(grep -w hugetlbfs /proc/filesystems 2>/dev/null | cut -f2)

	if [ "$fssupport" = "hugetlbfs" ]; then
		return 1
	else
		return 0
	fi
}

test6()
{
	cpuset_set "$CPUSET/0" "$cpu_of_node0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	check_hugetlbfs
	if [ $? -eq 0 ]; then
		tst_resm TCONF "This system don't support hugetlbfs"
		return 0
	fi

	mkdir /hugetlb
	mount -t hugetlbfs none /hugetlb

	save_nr_hugepages=$(cat /proc/sys/vm/nr_hugepages)
	echo $((2*$nr_mems)) > /proc/sys/vm/nr_hugepages

	cpuset_memory_test --shm --hugepage -s $HUGEPAGESIZE --key=7 >"$MEMORY_RESULT" &
	simple_getresult $! "$CPUSET/0"

	umount /hugetlb
	rmdir /hugetlb

	echo $save_nr_hugepages > /proc/sys/vm/nr_hugepages
	if [ $(cat /proc/sys/vm/nr_hugepages) -ne $save_nr_hugepages ]; then
		tst_resm TFAIL "can't restore nr_hugepages(nr_hugepages = $save_nr_hugepages)."
		return 1
	fi

	if [ "$node" != "0" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node(Expect: Node#0)."
		return 1
	fi
}

test7()
{
	cpuset_set "$CPUSET/0" "$cpu_of_node0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	cpuset_memory_test --mmap-anon >"$MEMORY_RESULT" &
	simple_getresult $! "$CPUSET/0"
	if [ "$node" != "0" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node(Expect: Node#0)."
		return 1
	fi
}

test8()
{
	cpuset_set "$CPUSET/0" "$cpu_of_node0" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	cpuset_memory_test --mmap-anon >"$MEMORY_RESULT" &
	simple_getresult $! "$CPUSET/0"
	if [ "$node" != "1" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node(Expect: Node#1)."
		return 1
	fi
}

# talk2memory_test_for_case_10_11
# $1 - cpuset_memory_test's pid
# $2 - move cpuset_memory_test's pid to this cpuset
# $3 - move cpuset_memory_test's pid to this cpuset(a new cpuset)
talk2memory_test_for_case_10_11()
{
	sleep 1
	echo $1 > "$2/tasks"
	/bin/kill -s SIGUSR1 $1
	sleep 1
	echo $1 > "$3/tasks"
	/bin/kill -s SIGUSR1 $1
	sleep 1
	/bin/kill -s SIGUSR1 $1
	sleep 1
	/bin/kill -s SIGINT $1
	wait $1
}

test9()
{
	cpuset_set "$CPUSET/1" "$cpus_all" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group1's parameter failed."
		return 1
	fi

	cpuset_set "$CPUSET/2" "$cpus_all" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's parameter failed."
		return 1
	fi

	cpuset_memory_test --mmap-anon --check >"$MEMORY_RESULT" &
	talk2memory_test_for_case_10_11 $! "$CPUSET/1" "$CPUSET/2"
	{
		read node0
		read node1
		read node2
	} < "$MEMORY_RESULT"

	if [ "$node0" != "0" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node0(Expect: Node#0)."
		return 1
	fi
	if [ "$node1" != "0" ]; then
		tst_resm TFAIL "Allocated memory was moved to the Node#$node1(Expect: Node#0)."
		return 1
	fi
	if [ "$node2" != "1" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node2(Expect: Node#1) after changing group."
		return 1
	fi
}

test10()
{
	cpuset_set "$CPUSET/1" "$cpus_all" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group1's parameter failed."
		return 1
	fi

	cpuset_set "$CPUSET/2" "$cpus_all" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's parameter failed."
		return 1
	fi

	echo 1 > "$CPUSET/2/cpuset.memory_migrate" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's memory_migrate failed."
		return 1
	fi

	cpuset_memory_test --mmap-anon --check >"$MEMORY_RESULT" &
	talk2memory_test_for_case_10_11 $! "$CPUSET/1" "$CPUSET/2"
	{
		read node0
		read node1
		read node2
	} < "$MEMORY_RESULT"

	if [ "$node0" != "0" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node0(Expect: Node#0)."
		return 1
	fi
	if [ "$node1" != "1" ]; then
		tst_resm TFAIL "Allocated memory was not moved to the Node#1(Result: Node#$node1)."
		return 1
	fi
	if [ "$node2" != "1" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node2(Expect: Node#1) after changing group."
		return 1
	fi
}


# talk2memory_test_for_case_12_13
# $1 - cpuset_memory_test's pid
# $2 - the test cpuset
talk2memory_test_for_case_12_13()
{
	sleep 1
	echo $1 > "$2/tasks"
	/bin/kill -s SIGUSR1 $1

	echo 0 > "$2/cpuset.mems" || return 1
	sleep 1
	/bin/kill -s SIGUSR1 $1
	sleep 1
	/bin/kill -s SIGUSR1 $1
	sleep 1
	/bin/kill -s SIGINT $1
	wait $1
}


test11()
{
	cpuset_set "$CPUSET/0" "$cpu_of_node0" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group's parameter failed."
		return 1
	fi

	cpuset_memory_test --mmap-anon >"$MEMORY_RESULT" &
	talk2memory_test_for_case_12_13 $! "$CPUSET/0"

	{
		read node0
		read node1
	} < "$MEMORY_RESULT"

	if [ "$node0" != "1" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node0(Expect: Node#1)."
		return 1
	fi
	if [ "$node1" != "0" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node1(Expect: Node#0) after changing mems."
		return 1
	fi
}


test12()
{
	cpuset_set "$CPUSET/0" "$cpu_of_node0" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group's parameter failed."
		return 1
	fi

	echo 1 > "$CPUSET/0/cpuset.memory_migrate" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group's memory_migrate failed."
		return 1
	fi


	cpuset_memory_test --mmap-anon --check >"$MEMORY_RESULT" &
	talk2memory_test_for_case_12_13 $! "$CPUSET/0"

	{
		read node0
		read node1
		read node2
	} < "$MEMORY_RESULT"

	if [ "$node0" != "1" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node0(Expect: Node#1)."
		return 1
	fi
	if [ "$node1" != "0" ]; then
		tst_resm TFAIL "Allocated memory was not moved to the Node#0(Result: Node#$node1)."
		return 1
	fi
	if [ "$node2" != "0" ]; then
		tst_resm TFAIL "allocate memory on the Node#$node2(Expect: Node#0) after changing mems."
		return 1
	fi
}

# get the second thread's tid
#$1 - pid
get_the_second()
{
	ls /proc/$1/task | (
		read tid1
		read tid2
		if [ "$1" -eq "$tid1" ]
		then
			echo $tid2
		else
			echo $tid1
		fi
	)
}

test13()
{
	cpuset_set "$CPUSET/1" "$cpu_of_node0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group1's parameter failed."
		return 1
	fi

	cpuset_set "$CPUSET/2" "$cpu_of_node0" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's parameter failed."
		return 1
	fi

	cpuset_memory_test --thread --mmap-anon >"$MEMORY_RESULT" &
	{
		local testpid=$!
		sleep 1
		local testtid=$(get_the_second $testpid)

		echo $testpid > "$CPUSET/1/tasks"
		/bin/kill -s SIGUSR1 $testpid

		echo $testtid > "$CPUSET/2/tasks"
		sleep 1
		/bin/kill -s SIGUSR2 $testpid
		sleep 1
		/bin/kill -s SIGINT $testpid
		wait $testpid
	}

	{
		read node0
		read node1
	} < "$MEMORY_RESULT"

	if [ "$node0" != "0" ]; then
		tst_resm TFAIL "Thread1 allocated memory on the Node#$node0(Expect: Node#0)."
		return 1
	fi
	if [ "$node1" != "1" ]; then
		tst_resm TFAIL "Thread2 allocated memory on the Node#$node1(Expect: Node#1)."
		return 1
	fi
}

test14()
{
	cpuset_set "$CPUSET/1" "$cpu_of_node0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group1's parameter failed."
		return 1
	fi

	cpuset_set "$CPUSET/2" "$cpu_of_node0" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's parameter failed."
		return 1
	fi

	echo 1 > "$CPUSET/2/cpuset.memory_migrate" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's memory_migrate failed."
		return 1
	fi


	cpuset_memory_test --thread --mmap-anon >"$MEMORY_RESULT" &
	{
		local testpid=$!
		sleep 1
		local testtid=$(get_the_second $testpid)

		echo $testpid > "$CPUSET/1/tasks"
		/bin/kill -s SIGUSR1 $testpid

		echo $testtid > "$CPUSET/2/tasks"
		sleep 1
		/bin/kill -s SIGUSR2 $testpid
		sleep 1
		/bin/kill -s SIGINT $testpid
		wait $testpid
	}

	{
		read node0
		read node1
	} < "$MEMORY_RESULT"

	if [ "$node0" != "0" ]; then
		tst_resm TFAIL "Thread1 allocated memory on the Node#$node0(Expect: Node#0)."
		return 1
	fi
	if [ "$node1" != "1" ]; then
		tst_resm TFAIL "Thread2 allocated memory on the Node#$node1(Expect: Node#1)."
		return 1
	fi
}

test15()
{
	cpuset_set "$CPUSET/1" "$cpu_of_node0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group1's parameter failed."
		return 1
	fi

	cpuset_set "$CPUSET/2" "$cpu_of_node0" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's parameter failed."
		return 1
	fi

	echo 1 > "$CPUSET/2/cpuset.memory_migrate" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's memory_migrate failed."
		return 1
	fi


	cpuset_memory_test --thread --mmap-anon >"$MEMORY_RESULT" &
	{
		local testpid=$!
		sleep 1
		local testtid=$(get_the_second $testpid)

		echo $testpid > "$CPUSET/1/tasks"
		/bin/kill -s SIGUSR1 $testpid

		echo $testtid > "$CPUSET/2/tasks"
		sleep 1
		echo 1 > "$CPUSET/1/cpuset.memory_migrate"
		sleep 1
		/bin/kill -s SIGUSR2 $testpid
		sleep 1
		/bin/kill -s SIGUSR1 $testpid
		sleep 1
		/bin/kill -s SIGUSR1 $testpid
		sleep 1
		/bin/kill -s SIGINT $testpid
		wait $testpid
	}

	{
		read node0
		read node1
		read node2
	} < "$MEMORY_RESULT"

	if [ "$node0" != "0" ]; then
		tst_resm TFAIL "Thread1 allocated memory on the Node#$node0(Expect: Node#0) first."
		return 1
	fi
	if [ "$node1" != "1" ]; then
		tst_resm TFAIL "Thread2 allocated memory on the Node#$node1(Expect: Node#1)."
		return 1
	fi
	if [ "$node2" != "0" ]; then
		tst_resm TFAIL "Thread1 allocated memory on the Node#$node2(Expect: Node#0) second."
		return 1
	fi
}

test16()
{
	cpuset_set "$CPUSET/1" "$cpu_of_node0" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group1's parameter failed."
		return 1
	fi

	cpuset_set "$CPUSET/2" "$cpu_of_node0" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's parameter failed."
		return 1
	fi

	echo 1 > "$CPUSET/2/cpuset.memory_migrate" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's memory_migrate failed."
		return 1
	fi

	cpuset_memory_test --thread --mmap-anon >"$MEMORY_RESULT" &
	{
		local testpid=$!
		sleep 1
		local testtid=$(get_the_second $testpid)

		echo $testpid > "$CPUSET/1/tasks"
		/bin/kill -s SIGUSR1 $testpid

		echo $testtid > "$CPUSET/2/tasks"
		sleep 1
		echo 0 > "$CPUSET/1/cpuset.mems"
		sleep 1
		/bin/kill -s SIGUSR2 $testpid
		sleep 1
		/bin/kill -s SIGUSR1 $testpid
		sleep 1
		/bin/kill -s SIGUSR1 $testpid
		sleep 1
		/bin/kill -s SIGUSR2 $testpid
		sleep 1
		/bin/kill -s SIGUSR2 $testpid
		sleep 1
		/bin/kill -s SIGINT $testpid
		wait $testpid
	}

	{
		read node0
		read node1
		read node2
		read node3
	} < "$MEMORY_RESULT"

	if [ "$node0" != "1" ]; then
		tst_resm TFAIL "Thread1 allocated memory on the Node#$node0(Expect: Node#1) first."
		return 1
	fi
	if [ "$node1" != "1" ]; then
		tst_resm TFAIL "Thread2 allocated memory on the Node#$node1(Expect: Node#1) first."
		return 1
	fi
	if [ "$node2" != "0" ]; then
		tst_resm TFAIL "Thread1 allocated memory on the Node#$node2(Expect: Node#0) second"
		return 1
	fi
	if [ "$node3" != "1" ]; then
		tst_resm TFAIL "Thread2 allocated memory on the Node#$node3(Expect: Node#1) second"
		return 1
	fi
}

test17()
{
	cpuset_set "$CPUSET/1" "$cpu_of_node0" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group1's parameter failed."
		return 1
	fi

	echo 1 > "$CPUSET/1/cpuset.memory_migrate" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group1's memory_migrate failed."
		return 1
	fi

	cpuset_set "$CPUSET/2" "$cpu_of_node0" "1" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's parameter failed."
		return 1
	fi

	echo 1 > "$CPUSET/2/cpuset.memory_migrate" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2's memory_migrate failed."
		return 1
	fi

	cpuset_memory_test --thread --mmap-anon >"$MEMORY_RESULT" &
	{
		local testpid=$!
		sleep 1
		local testtid=$(get_the_second $testpid)

		echo $testpid > "$CPUSET/1/tasks"
		/bin/kill -s SIGUSR1 $testpid

		echo $testtid > "$CPUSET/2/tasks"
		sleep 1
		echo 0 > "$CPUSET/1/cpuset.mems"
		sleep 1
		/bin/kill -s SIGUSR2 $testpid
		sleep 1
		/bin/kill -s SIGUSR1 $testpid
		sleep 1
		/bin/kill -s SIGUSR1 $testpid
		sleep 1
		/bin/kill -s SIGUSR2 $testpid
		sleep 1
		/bin/kill -s SIGUSR2 $testpid
		sleep 1
		/bin/kill -s SIGINT $testpid
		wait $testpid
	}

	{
		read node0
		read node1
		read node2
		read node3
	} < "$MEMORY_RESULT"

	if [ "$node0" != "1" ]; then
		tst_resm TFAIL "Thread1 allocated memory on the Node#$node0(Expect: Node#1) first."
		return 1
	fi
	if [ "$node1" != "1" ]; then
		tst_resm TFAIL "Thread2 allocated memory on the Node#$node1(Expect: Node#1) first."
		return 1
	fi
	if [ "$node2" != "0" ]; then
		tst_resm TFAIL "Thread1 allocated memory on the Node#$node2(Expect: Node#0) second"
		return 1
	fi
	if [ "$node3" != "1" ]; then
		tst_resm TFAIL "Thread2 allocated memory on the Node#$node3(Expect: Node#1) second"
		return 1
	fi
}

for c in $(seq 1 $TST_TOTAL)
do
	setup
	if [ $? -ne 0 ]; then
		exit_status=1
	else
		test$c
		if [ $? -ne 0 ]; then
			exit_status=1
			cleanup
		else
			cleanup
			if [ $? -ne 0 ]; then
				exit_status=1
			else
				tst_resm TPASS "Cpuset memory allocation test succeeded."
			fi
		fi
	fi
	TST_COUNT=$(($TST_COUNT + 1))
done

exit $exit_status

