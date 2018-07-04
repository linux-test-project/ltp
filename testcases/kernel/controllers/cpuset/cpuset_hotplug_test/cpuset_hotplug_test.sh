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

export TCID="cpuset_hotplug"
export TST_TOTAL=13
export TST_COUNT=1

. cpuset_funcs.sh

check 2 1

exit_status=0

nr_cpus=$NR_CPUS
nr_mems=$N_NODES

cpus_all="$(seq -s, 0 $((nr_cpus-1)))"
cpus_all="`cpuset_list_compute $cpus_all`"
mems_all="$(seq -s, 0 $((nr_mems-1)))"

# check_result <result> <expect>
check_result()
{
	local result="$1"
	local expect="$2"

	case "$expect" in
	EMPTY)
		test -z "$result"
		return $?
		;;
	*)
		test "$expect" = "$result"
		return $?
		;;
	esac
}

# root_cpu_hotplug_test <cpuhotplug> <expect_cpus> <expect_task_cpus>
root_cpu_hotplug_test()
{
	local cpuhotplug="$1"
	local expect_cpus="$2"
	local expect_task_cpus="$3"

	local root_cpus=
	local task_cpus=
	local tst_pid=
	local ret=

	setup_test_environment $cpuhotplug 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "setup test environment(offline CPU#$HOTPLUG_CPU) failed"
		return 1
	fi

	/bin/cat /dev/zero > /dev/null 2>&1 &
	tst_pid=$!

	cpu_hotplug $HOTPLUG_CPU $cpuhotplug 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "$cpuoffline CPU#$HOTPLUG_CPU failed."
		return 1
	fi

	root_cpus="`cat $CPUSET/cpuset.cpus`"

	task_cpus="`cat /proc/$tst_pid/status | grep Cpus_allowed_list`"
	task_cpus="`echo $task_cpus | sed -e 's/Cpus_allowed_list: //'`"

	check_result "$root_cpus" "$expect_cpus"
	ret=$?
	if [ $ret -eq 0 ]
	then
		check_result "$task_cpus" "$expect_task_cpus"
		ret=$?
		if [ $ret -ne 0 ]; then
			tst_resm TFAIL "task's allowed list isn't expected.(Result: $task_cpus, Expect: $expect_task_cpus)"
		fi
	else
		tst_resm TFAIL "root group's cpus isn't expected(Result: $root_cpus, Expect: $expect_cpus)."
	fi

	/bin/kill -9 $tst_pid > /dev/null 2>&1

	return $ret
}

# general_cpu_hotplug_test <cpuhotplug> <cpus> <expect_cpus> <expect_task_cpus>
general_cpu_hotplug_test()
{
	local cpuhotplug="$1"
	local cpus="$2"
	local expect_cpus="$3"
	local expect_task_cpus="$4"
	local path="$CPUSET/1"

	local tst_pid=
	local task_cpus=
	local ret=

	setup_test_environment $cpuhotplug 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "setup test environment(offline CPU#$HOTPLUG_CPU) failed"
		return 1
	fi

	cpuset_set "$path" "$cpus" "$mems_all" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	/bin/cat /dev/zero > /dev/null 2>&1 &
	tst_pid=$!

	echo $tst_pid > "$path/tasks" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "attach test tasks to group failed."
		/bin/kill -s SIGKILL $tst_pid
		return 1
	fi

	cpu_hotplug $HOTPLUG_CPU $cpuhotplug 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "$cpuoffline CPU#$HOTPLUG_CPU failed."
		/bin/kill -s SIGKILL $tst_pid
		return 1
	fi

	cpus="`cat $path/cpuset.cpus`"

	task_cpus="`cat /proc/$tst_pid/status | grep Cpus_allowed_list`"
	task_cpus="`echo $task_cpus | sed -e 's/Cpus_allowed_list: //'`"

	if [ "$expect_cpus" = "EMPTY" ]; then
		local tasks=`cat $path/tasks | grep "\b$tst_pid\b"`
		check_result "$tasks" "EMPTY"
		if [ $? -ne 0 ]; then
			tst_resm TFAIL "test task was still in general group, but its cpus is NULL"
			/bin/kill -s SIGKILL $tst_pid
			return 1
		fi

		tasks=`cat $CPUSET/tasks | grep "\b$tst_pid\b"`
		check_result "$tasks" "$tst_pid"
		if [ $? -ne 0 ]; then
			tst_resm TFAIL "test task wasn't moved to parent group"
			/bin/kill -s SIGKILL $tst_pid
			return 1
		fi
	fi

	check_result "$cpus" "$expect_cpus"
	ret=$?
	if [ $ret -eq 0 ]; then
		check_result $task_cpus $expect_task_cpus
		ret=$?
		if [ $ret -ne 0 ]; then
			tst_resm TFAIL "task's cpu allowed list isn't expected(Result: $task_cpus, Expect: $expect_task_cpus)."
		fi
	else
		if [ "$cpus" = "" ]; then
			cpus="EMPTY"
		fi
		tst_resm TFAIL "general group's cpus isn't expected(Result: $cpus, Expect: $expect_cpus)."
	fi
	/bin/kill -s SIGKILL $tst_pid > /dev/null 2>&1

	return $ret
}

base_test()
{
	setup
	if [ $? -ne 0 ]; then
		exit_status=1
	else
		"$test_function" "$@"
		if [ $? -ne 0 ]; then
			exit_status=1
			cleanup
		else
			cleanup
			if [ $? -ne 0 ]; then
				exit_status=1
			else
				tst_resm TPASS "Cpuset vs CPU hotplug test succeeded."
			fi
		fi

		cpu_hotplug_cleanup
	fi
	TST_COUNT=$(($TST_COUNT + 1))
}

# Test Case 1-2
test_root_cpu_hotplug()
{
	local tmp_cpus="`cpuset_list_compute -s $cpus_all $HOTPLUG_CPU`"

	test_function="root_cpu_hotplug_test"
	while read hotplug cpus_expect task_expect
	do
		base_test "$hotplug" "$cpus_expect" "$task_expect"
	done <<- EOF
		offline	$tmp_cpus	$cpus_all
		online	$cpus_all	$cpus_all
	EOF
	# while read hotplug cpus_expect task_expect
}

# Test Case 3-6
test_general_cpu_hotplug()
{
	local tmp_cpus="`cpuset_list_compute -s $cpus_all $HOTPLUG_CPU`"

	test_function="general_cpu_hotplug_test"
	while read hotplug cpus cpus_expect task_expect
	do
		base_test "$hotplug" "$cpus" "$cpus_expect" "$task_expect"
	done <<- EOF
		offline		0-1	0	0
		offline		1	EMPTY	$cpus_all
		offline		0	0	0
		online		0	0	0
	EOF
	# while read hotplug cpus cpus_expect task_expect
}

test_root_cpu_hotplug
test_general_cpu_hotplug

exit $exit_status
