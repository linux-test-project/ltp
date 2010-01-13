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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA      #
#                                                                              #
# Author: Miao Xie <miaox@cn.fujitsu.com>                                      #
#                                                                              #
################################################################################

cd $LTPROOT/testcases/bin

. ./cpuset_funcs.sh

export TCID="cpuset05"
export TST_TOTAL=16
export TST_COUNT=1

exit_status=0

TEST_CPUSET="$CPUSET/0"
TEST_OUTPUT="$CPUSET_TMP/result"
TEST_PROCSTATUS="$CPUSET_TMP/status"
TEST_PROCNUMA="$CPUSET_TMP/numa_maps"

# do_syscall_test - call ./syscall_test
# $1 - cpus
# $2 - mems
# $3 - ./syscall_test's args
# $4 - expect return value of test task

do_syscall_test()
{
	local testpid=
	local ret=

	mkdir -p "$TEST_CPUSET"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "mkdir -p $TEST_CPUSET fail."
		return 1
	fi
	echo "$1" > "$TEST_CPUSET/cpus"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "set $TEST_CPUSET/cpus as $1 fail."
		return 1
	fi
	echo "$2" > "$TEST_CPUSET/mems"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "set $TEST_CPUSET/mems as $2 fail."
		return 1
	fi
	./cpuset_syscall_test $3 >"$TEST_OUTPUT" &
	testpid=$!
	echo $testpid > "$TEST_CPUSET/tasks"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Attaching test task into group fail."
		return 1
	fi
	sleep 1
	/bin/kill -s SIGUSR1 $testpid
	sleep 1
	cat /proc/$testpid/status > $TEST_PROCSTATUS
	cat /proc/$testpid/numa_maps > $TEST_PROCNUMA
	/bin/kill -s SIGINT $testpid
	wait $testpid
	ret=$?
	if [ $4 -eq 0 ]; then
		if [ $ret -ne 0 ]; then
			tst_resm TFAIL "Test task exited abnormally.(expect return value is 0)"
			return 1
		fi
	else
		if [ $ret -eq 0 ]; then
			tst_resm TFAIL "Test task exited abnormally.(expect return value is !0)"
			return 1
		fi
	fi
	return 0
}

test1()
{
	do_syscall_test 0 0 --setaffinity=1 0 || return 1
	allowed_list="$(awk '/Cpus_allowed_list:/{print $2}' $TEST_PROCSTATUS )"
	test "$allowed_list" = "0"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"0\""
		return 1
	fi
	return 0
}

test2()
{
	do_syscall_test 0-1 0 --setaffinity=1 0 || return 1
	allowed_list="$(awk '/Cpus_allowed_list:/{print $2}' $TEST_PROCSTATUS )"
	test "$allowed_list" = "0"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"0\""
		return 1
	fi
	return 0
}

test3()
{
	do_syscall_test 0-1 0 --setaffinity=6 0 || return 1
	allowed_list="$(awk '/Cpus_allowed_list:/{print $2}' $TEST_PROCSTATUS )"
	test "$allowed_list" = "1"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"1\""
		return 1
	fi
	return 0
}

test4()
{
	do_syscall_test 0-1 0 --setaffinity=12 1 || return 1
	allowed_list="$(awk '/Cpus_allowed_list:/{print $2}' $TEST_PROCSTATUS )"
	test "$allowed_list" = "0-1" || return 1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"0-1\""
		return 1
	fi
	return 0
}

test5()
{
	do_syscall_test 0 0 --getaffinity 0 || return 1
	allowed_list="$(awk '/Cpus_allowed_list:/{print $2}' $TEST_PROCSTATUS )"
	test "$(cat "$TEST_OUTPUT")" = "0,"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(getaffinity) = \"$(cat $TEST_OUTPUT)\", expect = \"0,\")"
		return 1
	fi
	test "$allowed_list" = "0"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"0\")"
		return 1
	fi
	return 0
}

test6()
{
	do_syscall_test 0-1 0 --getaffinity 0 || return 1
	allowed_list="$(awk '/Cpus_allowed_list:/{print $2}' $TEST_PROCSTATUS )"
	test "$(cat "$TEST_OUTPUT")" = "0,1,"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(getaffinity) = \"$(cat $TEST_OUTPUT)\", expect = \"0,1,\")"
		return 1
	fi
	test "$allowed_list" = "0-1"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"0-1\")"
		return 1
	fi
	return 0
}

test7()
{
	do_syscall_test 0 0 --mbind=1 0 || return 1
	memory_addr="$(cat $TEST_OUTPUT)"
	memory_addr=${memory_addr##*0x}
	allowed_list=$(cat $TEST_PROCNUMA | grep "$memory_addr" | \
			awk '{print $2}')
	allowed_list="$(echo $allowed_list | sed -e s/bind://)"
	test "$allowed_list" = "0" || return 1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"0\")"
		return 1
	fi
	return 0
}

test8()
{
	do_syscall_test 0 0-1 --mbind=1 0 || return 1
	memory_addr="$(cat $TEST_OUTPUT)"
	memory_addr=${memory_addr##*0x}
	allowed_list=$(cat $TEST_PROCNUMA | grep "$memory_addr" | \
			awk '{print $2}')
	allowed_list="$(echo $allowed_list | sed -e s/bind://)"
	test "$allowed_list" = "0"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"0\")"
		return 1
	fi
	return 0
}

test9()
{
	do_syscall_test 0 0-1 --mbind=6 0 || return 1
	memory_addr="$(cat $TEST_OUTPUT)"
	memory_addr=${memory_addr##*0x}
	allowed_list=$(cat $TEST_PROCNUMA | grep "$memory_addr" | \
			awk '{print $2}')
	allowed_list="$(echo $allowed_list | sed -e s/bind://)"
	test "$allowed_list" = "1"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"1\")"
		return 1
	fi
	return 0
}

test10()
{
	do_syscall_test 0 0 --mbind=6 1 || return 1
	memory_addr="$(cat $TEST_OUTPUT)"
	memory_addr=${memory_addr##*0x}
	allowed_list=$(cat $TEST_PROCNUMA | grep "$memory_addr" | \
			awk '{print $2}')
	allowed_list="$(echo $allowed_list | sed -e s/bind://)"
	test "$allowed_list" = "default"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"default\")"
		return 1
	fi
	return 0
}

# this function is used by case 11-16
# check_result <expect>
check_result()
{
	local expect=$1
	while read allowed_list; do
		allowed_list="$(echo $allowed_list | awk '{print $2}')"
		allowed_list="$(echo $allowed_list | sed -e s/bind://)"
		test "$allowed_list" = "$expect"
		if [ $? -ne 0 ]; then
			tst_resm TFAIL "Result(/proc/<pid>/numa_maps) = \"$allowed_list\", expect = \"$expect\")"
			return 1
		fi
	done < $TEST_PROCNUMA
	return 0
}

test11()
{
	do_syscall_test 0 0 --set_mempolicy=1 0 || return 1
	check_result "0"
	return $?
}

test12()
{
	do_syscall_test 0 0-1 --set_mempolicy=1 0 || return 1
	check_result "0"
	return $?
}

test13()
{
	do_syscall_test 0 0-1 --set_mempolicy=6 0 || return 1
	check_result "1"
	return $?
}

test14()
{
	do_syscall_test 0 0 --set_mempolicy=6 1 || return 1
	check_result "default"
	return $?
}

test15()
{
	do_syscall_test 0 0 --get_mempolicy 0 || return 1
	allowed_list="$(awk '/Mems_allowed_list:/{print $2}' $TEST_PROCSTATUS )"
	test "$(cat "$TEST_OUTPUT")" = "0"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(get_mempolicy) = \"$(cat $TEST_OUTPUT)\", expect = \"1\")"
		return 1
	fi
	test "$allowed_list" = "0"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"0\")"
		return 1
	fi
	check_result "default" || return 1
	return 0
}

test16()
{
	do_syscall_test 0 0-1 --get_mempolicy 0 || return 1
	allowed_list="$(awk '/Mems_allowed_list:/{print $2}' $TEST_PROCSTATUS )"
	test "$(cat "$TEST_OUTPUT")" = "0-1"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(get_mempolicy) = \"$(cat $TEST_OUTPUT)\", expect = \"3\")"
		return 1
	fi
	test "$allowed_list" = "0-1"
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Result(/proc/<pid>/status) = \"$allowed_list\", expect = \"0-1\")"
		return 1
	fi
	check_result "default" || return 1
	return 0
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
				tst_resm TPASS "Cpuset vs systemcall test succeeded."
			fi
		fi
	fi
	: $((TST_COUNT++))
done

exit $exit_status

