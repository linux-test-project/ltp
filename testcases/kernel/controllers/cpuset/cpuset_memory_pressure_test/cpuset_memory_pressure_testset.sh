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

export TCID="cpuset_memory_pressure"
export TST_TOTAL=6
export TST_COUNT=1

. cpuset_funcs.sh

check

exit_status=0

# usable physical memory
py_mem=$(free -m | awk '{if(NR==2) print $4 + $6 + $7}')

# free swap space
sw_mem=$(free -m | awk '{if(NR==4) print $4}')

# the memory which is going to be used
usemem=$((py_mem - 20))

test1()
{
	echo 0 > "$CPUSET/cpuset.memory_pressure_enabled" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "Closing memory_pressure_enabled failed."
		return 1
	fi

	local i
	for i in $(seq 0 9)
	do
		if [ $(cat "$CPUSET/cpuset.memory_pressure") -ne 0 ]; then
			tst_resm TFAIL "Memory_pressure had memory pressure rate."
			return 1
		fi
	done
}

test2()
{
	echo 0 > "$CPUSET/cpuset.memory_pressure_enabled" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "Closing memory_pressure_enabled failed."
		return 1
	fi

	cpuset_memory_pressure $usemem

	local i
	for i in $(seq 0 9)
	do
		if [ $(cat "$CPUSET/cpuset.memory_pressure") -ne 0 ]; then
			tst_resm TFAIL "Memory_pressure had memory pressure rate."
			return 1
		fi
	done
}

test3()
{
	echo 1 > "$CPUSET/cpuset.memory_pressure_enabled" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "Opening memory_pressure_enabled failed."
		return 1
	fi

	cpuset_set "$CPUSET/sub_cpuset" "0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	local i
	for i in $(seq 0 9)
	do
		if [ $(cat "$CPUSET/cpuset.memory_pressure") -ne 0 ]; then
			tst_resm TFAIL "root group's memory_pressure had memory pressure rate."
			return 1
		fi
		if [ $(cat "$CPUSET/sub_cpuset/cpuset.memory_pressure") -ne 0 ]; then
			tst_resm TFAIL "sub group's memory_pressure had memory pressure rate."
			return 1
		fi
	done
	return 0
}

test4()
{
	echo 1 > "$CPUSET/cpuset.memory_pressure_enabled" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "Opening memory_pressure_enabled failed."
		return 1
	fi

	cpuset_set "$CPUSET/sub_cpuset" "0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	echo $$ > "$CPUSET/sub_cpuset/tasks" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "attaching self to sub group failed"
		return 1
	fi

	cpuset_memory_pressure $usemem

	echo $$ > "$CPUSET/tasks" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "moving self to root group failed"
		return 1
	fi

	local i
	for i in $(seq 0 9)
	do
		if [ $(cat "$CPUSET/cpuset.memory_pressure") -ne 0 ]; then
			tst_resm TFAIL "root group's memory_pressure had memory pressure rate."
			return 1
		fi
		if [ $(cat "$CPUSET/sub_cpuset/cpuset.memory_pressure") -eq 0 ]; then
			tst_resm TFAIL "sub group's memory_pressure didn't have memory pressure rate."
			return 1
		fi
	done
}

test5()
{
	echo 1 > "$CPUSET/cpuset.memory_pressure_enabled" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "Opening memory_pressure_enabled failed."
		return 1
	fi

	cpuset_set "$CPUSET/sub_cpuset" "0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	cpuset_memory_pressure $usemem
	local i
	for i in $(seq 0 9)
	do
		if [ $(cat "$CPUSET/cpuset.memory_pressure") -eq 0 ]; then
			tst_resm TFAIL "root group's memory_pressure didn't have memory pressure rate."
			return 1
		fi
		if [ $(cat "$CPUSET/sub_cpuset/cpuset.memory_pressure") -ne 0 ]; then
			tst_resm TFAIL "sub group's memory_pressure had memory pressure rate."
			return 1
		fi
	done
}

test6()
{
	echo 1 > "$CPUSET/cpuset.memory_pressure_enabled" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "Opening memory_pressure_enabled failed."
		return 1
	fi

	cpuset_set "$CPUSET/sub_cpuset" "0" "0" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	echo $$ > "$CPUSET/sub_cpuset/tasks" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "attaching self to sub group failed"
		return 1
	fi
	cpuset_memory_pressure $usemem

	echo $$ > "$CPUSET/tasks" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "attaching self to root group failed"
		return 1
	fi

	cpuset_memory_pressure $usemem
	local i
	for i in $(seq 0 9)
	do
		if [ $(cat "$CPUSET/cpuset.memory_pressure") -eq 0 ]; then
			tst_resm TFAIL "root group's memory_pressure didn't have memory pressure rate."
			return 1
		fi
		if [ $(cat "$CPUSET/sub_cpuset/cpuset.memory_pressure") -eq 0 ]; then
			tst_resm TFAIL "sub group's memory_pressure didn't have memory pressure rate."
			return 1
		fi
	done
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
				tst_resm TPASS "Cpuset memory pressure test succeeded."
			fi
		fi
	fi
	TST_COUNT=$(($TST_COUNT + 1))
done

exit $exit_status
