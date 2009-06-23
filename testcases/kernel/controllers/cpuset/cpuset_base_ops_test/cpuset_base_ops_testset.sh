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

export TCID="cpuset01"
export TST_TOTAL=97
export TST_COUNT=1

nr_cpus=$NR_CPUS
nr_mems=$N_NODES

cpus_all="$(seq -s, 0 $((nr_cpus-1)))"
mems_all="$(seq -s, 0 $((nr_mems-1)))"

exit_status=0

cfile_name=

# base_op_write_and_test <write_file_name> <write_string> <expect_string>
base_op_write_and_test()
{
	local write_file="$1"
	local write_string="$2"
	local expect_string="$3"
	local write_result=
	local ret=0

	mkdir -p "$(dirname $write_file)" || {
		tst_brkm TFAIL "Failed to mkdir -p $(basename $write_file)"
		return 1
	}
	[ "$write_string" = NULL ] && write_string=" "
	
	/bin/echo "$write_string" > "$write_file" 2> $CPUSET_TMP/stderr
	ret=$?
	write_result="$(cat "$write_file")"
	
	case "$expect_string" in
	EMPTY)
		test -z "$write_result" -a $ret = 0
		ret=$?
		;;
	WRITE_ERROR)
		ret=$((!$ret))
		;;
	*)
		test "$expect_string" = "$write_result" -a $ret = 0
		ret=$?
		;;
	esac

	if [ $ret -eq 0 ]; then
		tst_resm TPASS "$cfile_name: Get the expected string"
	else
		tst_resm TFAIL "$cfile_name: Test result - $write_result Expected string - \"$expect_string\""
	fi
	return $ret
}

base_op_test()
{
	setup
	if [ $? -ne 0 ]; then
		exit_status=1
	else
		base_op_write_and_test "$@"
		if [ $? -ne 0 ]; then
			exit_status=1
		fi

		cleanup
		if [ $? -ne 0 ]; then
			exit_status=1
		fi
	fi
	: $((TST_COUNT++))
}

test_cpus()
{
	cfile_name="cpus"
	while read cpus result
	do
		base_op_test "$CPUSET/1/cpus" "$cpus" "$result"
	done <<- EOF
		NULL					EMPTY
		0					0
		$nr_cpus				WRITE_ERROR
		$cpus_all				0-$((nr_cpus-1))
		${cpus_all}$nr_cpus			WRITE_ERROR
		0,0					0
		0-0					0
		0-$((nr_cpus-1))			0-$((nr_cpus-1))
		-1					WRITE_ERROR
		0-$nr_cpus				WRITE_ERROR
		0-					WRITE_ERROR
		0--$((nr_cpus-1))			WRITE_ERROR
		0,1-$((nr_cpus-2)),$((nr_cpus-1))	0-$((nr_cpus-1))
		0,1-$((nr_cpus-2)),			0-$((nr_cpus-2))
		0AAA					WRITE_ERROR
		AAA					WRITE_ERROR
	EOF
	# while read cpus result
}

test_mems()
{
	cfile_name="mems"
	while read mems result
	do
		base_op_test "$CPUSET/1/mems" "$mems" "$result"
	done <<- EOF
		NULL					EMPTY
		0					0
		$nr_mems				WRITE_ERROR
		$mems_all				0-$((nr_mems-1))
		${mems_all}$nr_mems			WRITE_ERROR
		0,0					0
		0-0					0
		0-$((nr_mems-1))			0-$((nr_mems-1))
		-1					WRITE_ERROR
		0-$nr_mems				WRITE_ERROR
		0-					WRITE_ERROR
		0--$((nr_mems-1))			WRITE_ERROR
		0,1-$((nr_mems-2)),$((nr_mems-1))	0-$((nr_mems-1))
		0,1-$((nr_mems-2)),			0-$((nr_mems-2))
		0AAA					WRITE_ERROR
		AAA					WRITE_ERROR
	EOF
	# while read mems result
}

test_flags()
{
	for filename in cpu_exclusive mem_exclusive mem_hardwall \
			memory_migrate memory_spread_page memory_spread_slab \
			sched_load_balance memory_pressure_enabled
	do
		cfile_name="$filename"
		while read flags result
		do
			base_op_test "$CPUSET/$filename" "$flags" "$result"
		done <<- EOF
			NULL	0
			0	0
			1	1
			-1	WRITE_ERROR
			A	WRITE_ERROR
			2	1
		EOF
		# while read flags, result
	done # for filename in flagfiles
}

test_domain()
{
	cfile_name="sched_relax_domain_level"
	while read domain_level result
	do
		base_op_test "$CPUSET/sched_relax_domain_level" "$domain_level" "$result"
	done <<- EOF
		NULL	0
		0	0
		1	1
		2	2
		3	3
		4	4
		5	5
		6	WRITE_ERROR
		-1	-1
		-2	WRITE_ERROR
		A	WRITE_ERROR
	EOF
	# while read domain_level result
}

# attach_task_test <cpus> <mems> <expect>
attach_task_test()
{
	local cpus=$1
	local mems=$2
	local expect=$3

	local pid=
	local ret=

	setup
	if [ $? -ne 0 ]; then
		exit_status=1
		cleanup
		: $((TST_COUNT++))
		return
	fi

	# create sub cpuset
	mkdir "$CPUSET/sub_cpuset" > /dev/null
	if [ $? -ne 0 ]; then
		exit_status=1
		cleanup
		: $((TST_COUNT++))
		return
	fi

	if [ "$cpus" != "NULL" ]; then
		echo $cpus > "$CPUSET/sub_cpuset/cpus"
	fi
	if [ "$mems" != "NULL" ]; then
		echo $mems > "$CPUSET/sub_cpuset/mems"
	fi

	cat /dev/zero > /dev/null &
	pid=$!

	# attach task into the cpuset group
	echo $pid > "$CPUSET/sub_cpuset/tasks" 2> /dev/null
	if [ $? -eq $expect ]; then
		tst_resm TPASS "Attaching Task Test successed!!"
	else
		tst_resm TFAIL "Attaching Task Test failed!! cpus - \"$cpus\", mems - \"$mems\", Expect - \"$expect\", Fact - \"$ret\". (0 - Attach Success, 1 - Attach Fail)"
		exit_status=1
	fi

	/bin/kill $pid > /dev/null 2>&1
	cleanup
	if [ $? -ne 0 ]; then
		exit_status=1
	fi
	: $((TST_COUNT++))
}


test_attach_task()
{
	cfile_name="tasks"
	while read cpus mems expect
	do
		attach_task_test "$cpus" "$mems" "$expect"
	done <<- EOF
		0	NULL	1
		0	0	0
		NULL	0	1
	EOF
	# while read cpus mems expect
}

test_readonly_cfiles()
{
	for filename in cpus mems memory_pressure
	do
		cfile_name="$filename(READONLY)"
		base_op_test "$CPUSET/$filename" "0" "WRITE_ERROR"
	done # for filename in readonly cfiles
}

# Case 1-3
test_readonly_cfiles

# Case 4-19
test_cpus

# Case 20-35
test_mems

# Case 36-83
test_flags

# Case 84-94
test_domain

# Case 95-97
test_attach_task

exit $exit_status
