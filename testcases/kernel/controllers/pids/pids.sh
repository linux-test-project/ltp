#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015 SUSE
# Author: Cedric Hnyda <chnyda@suse.com>
# Usage
# ./pids.sh caseno max

TST_CLEANUP=cleanup
TST_SETUP=setup
TST_TESTFUNC=do_test
TST_POS_ARGS=3
TST_USAGE=usage
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="killall"

caseno=$1
max=$2
subcgroup_num=$3
mounted=1

usage()
{
	cat << EOF
usage: $0 caseno max_processes

caseno        - testcase number from interval 1-9
max_processes - maximal number of processes to attach
                (not applicable to testcase 5)
subcgroup_num - number of subgroups created in group
		(only applicable to testcase 7)
OPTIONS
EOF
}

cleanup()
{
	killall -9 pids_task2 >/dev/null 2>&1

	cgroup_cleanup
}

setup()
{
	cgroup_require "pids"
	cgroup_version=$(cgroup_get_version "pids")
	testpath=$(cgroup_get_test_path "pids")
	task_list=$(cgroup_get_task_list "pids")

	tst_res TINFO "test starts with cgroup version $cgroup_version"
}

start_pids_tasks2()
{
	start_pids_tasks2_path $testpath $1
}

start_pids_tasks2_path()
{
	path=$1
	nb=$2
	for i in `seq 1 $nb`; do
		pids_task2 &
		echo $! > "$path/$task_list"
	done

	if [ $(wc -l < "$path/$task_list") -ne "$nb" ]; then
		tst_brk TBROK "failed to attach process"
	fi
}

stop_pids_tasks()
{
	stop_pids_tasks_path $testpath
}

stop_pids_tasks_path()
{
	local i
	path=$1

	for i in $(cat "$path/$task_list"); do
		ROD kill -9 $i
		wait $i
	done
}

case1()
{
	start_pids_tasks2 $max

	# should return 0 because there is no limit
	pids_task1 "$testpath/$task_list"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_res TFAIL "fork failed unexpectedly"
	elif [ "$ret" -eq "0" ]; then
		tst_res TPASS "fork didn't fail"
	else
		tst_res TBROK "pids_task1 failed"
	fi

	stop_pids_tasks
}

case2()
{
	tmp=$((max - 1))
	tst_res TINFO "limit the number of pid to $max"
	ROD echo $max \> $testpath/pids.max

	start_pids_tasks2 $tmp

	# should return 2 because the limit of pids is reached
	pids_task1 "$testpath/$task_list"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_res TPASS "fork failed as expected"
	elif [ "$ret" -eq "0" ]; then
		tst_res TFAIL "fork didn't fail despite the limit"
	else
		tst_res TBROK "pids_task1 failed"
	fi

	stop_pids_tasks
}

case3()
{
	lim=$((max + 2))
	tst_res TINFO "limit the number of avalaible pid to $lim"
	ROD echo $lim \> $testpath/pids.max

	start_pids_tasks2 $max

	pids_task1 "$testpath/$task_list"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_res TFAIL "fork failed unexpectedly"
	elif [ "$ret" -eq "0" ]; then
		tst_res TPASS "fork worked as expected"
	else
		tst_res TBROK "pids_task1 failed"
	fi

	stop_pids_tasks
}

case4()
{
	tst_res TINFO "limit the number of avalaible pid to 0"
	ROD echo 0 \> $testpath/pids.max

	start_pids_tasks2 $max

	tst_res TPASS "all process were attached"

	stop_pids_tasks
}

case5()
{
	tst_res TINFO "try to limit the number of avalaible pid to -1"
	echo -1 > $testpath/pids.max

	if [ "$?" -eq "0" ]; then
		tst_res TFAIL "managed to set the limit to -1"
	else
		tst_res TPASS "didn't manage to set the limit to -1"
	fi
}

case6()
{
	tst_res TINFO "set a limit that is smaller than current number of pids"
	start_pids_tasks2 $max

	lim=$((max - 1))
	ROD echo $lim \> $testpath/pids.max

	pids_task1 "$testpath/$task_list"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_res TPASS "fork failed as expected"
	elif [ "$ret" -eq "0" ]; then
		tst_res TFAIL "fork didn't fail despite the limit"
	else
		tst_res TBROK "pids_task1 failed"
	fi

	stop_pids_tasks
}

case7()
{
	tst_res TINFO "the number of all child cgroup tasks larger than its parent limit"

	lim=$((max / subcgroup_num))
	if [ "$((lim * subcgroup_num))" -ne "$max" ]; then
		tst_res TWARN "input max value must be a multiplier of $subcgroup_num"
		return
	fi

	ROD echo $max \> $testpath/pids.max

	for i in `seq 1 $subcgroup_num`; do
		mkdir $testpath/child$i
		start_pids_tasks2_path $testpath/child$i $lim
	done

	pids_task1 "$testpath/$task_list"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_res TPASS "parent cgroup fork failed as expected"
	elif [ "$ret" -eq "0" ]; then
		tst_res TFAIL "parent cgroup fork didn't fail despite the limit"
	else
		tst_res TBROK "parent cgroup pids_task1 failed"
	fi

	for i in `seq 1 $subcgroup_num`; do
		pids_task1 "$testpath/child$i/$task_list"
		ret=$?

		if [ "$ret" -eq "2" ]; then
			tst_res TPASS "child$i cgroup fork failed as expected"
		elif [ "$ret" -eq "0" ]; then
			tst_res TFAIL "child$i cgroup fork didn't fail despite the limit"
		else
			tst_res TBROK "child$i cgroup pids_task1 failed"
		fi
	done

	for i in `seq 1 $subcgroup_num`; do
		stop_pids_tasks_path $testpath/child$i
		rmdir $testpath/child$i
	done

	stop_pids_tasks
}

case8()
{
	tst_res TINFO "set child cgroup limit smaller than its parent limit"
	ROD echo $max \> $testpath/pids.max
	if [ "$cgroup_version" = "2" ]; then
		ROD echo +pids \> "$testpath"/cgroup.subtree_control
	fi
	mkdir $testpath/child

	lim=$((max - 1))
	ROD echo $lim \> $testpath/child/pids.max
	tmp=$((max - 2))
	start_pids_tasks2_path $testpath/child $tmp

	pids_task1 "$testpath/child/$task_list"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_res TPASS "fork failed as expected"
	elif [ "$ret" -eq "0" ]; then
		tst_res TFAIL "fork didn't fail despite the limit"
	else
		tst_res TBROK "pids_task1 failed"
	fi

	stop_pids_tasks_path $testpath/child
	rmdir $testpath/child
}

case9()
{
	tst_res TINFO "migrate cgroup"
	lim=$((max - 1))

	if [ "$cgroup_version" = "2" ]; then
		ROD echo +pids \> "$testpath"/cgroup.subtree_control
	fi
	for i in 1 2; do
		mkdir $testpath/child$i
		ROD echo $max \> $testpath/child$i/pids.max
		start_pids_tasks2_path $testpath/child$i $lim
	done

	pid=`head -n 1 "$testpath/child1/$task_list"`;
	ROD echo $pid \> "$testpath/child2/$task_list"

	if grep -q "$pid" "$testpath/child2/$task_list"; then
		tst_res TPASS "migrate pid $pid from cgroup1 to cgroup2 as expected"
	else
		tst_res TPASS "migrate pid $pid from cgroup1 to cgroup2 failed"
	fi

	if [ $(cat "$testpath/child1/pids.current") -eq $((lim - 1)) ]; then
		tst_res TPASS "migrate child1 cgroup as expected"
	else
		tst_res TFAIL "migrate child1 cgroup failed"
	fi

	if [ $(cat "$testpath/child2/pids.current") -eq $((lim + 1)) ]; then
		tst_res TPASS "migrate child2 cgroup as expected"
	else
		tst_res TFAIL "migrate child2 cgroup failed"
	fi

	pids_task1 "$testpath/child1/$task_list"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_res TFAIL "child1 fork failed unexpectedly"
	elif [ "$ret" -eq "0" ]; then
		tst_res TPASS "child1 fork worked as expected"
	else
		tst_res TBROK "child1 pids_task1 failed"
	fi

	pids_task1 "$testpath/child2/$task_list"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_res TPASS "child2 fork failed as expected"
	elif [ "$ret" -eq "0" ]; then
		tst_res TFAIL "child2 fork didn't fail despite the limit"
	else
		tst_res TBROK "child2 pids_task1 failed"
	fi

	for i in 1 2; do
		stop_pids_tasks_path $testpath/child$i
		rmdir $testpath/child$i
	done
	stop_pids_tasks
}

do_test()
{
	tst_res TINFO "Running testcase $caseno with $max processes"
	case$caseno
}

. cgroup_lib.sh
tst_run
