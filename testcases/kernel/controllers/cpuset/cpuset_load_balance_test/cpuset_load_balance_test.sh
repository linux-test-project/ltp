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

export TCID="cpuset_load_balance"
export TST_TOTAL=13
export TST_COUNT=1

. cpuset_funcs.sh

check 4 2

exit_status=0

nr_cpus=$NR_CPUS
nr_mems=$N_NODES

cpus_all="$(seq -s, 0 $((nr_cpus-1)))"
mems_all="$(seq -s, 0 $((nr_mems-1)))"

runtime=30

#cpuset_set_opt <path> <cfile> <value>
cpuset_set_opt()
{
	local path=$1
	local cfile=$2
	local value=$3

	/bin/echo $value > $path/$cfile
	return $?
}

# general_load_balance_test1 <g_cpus> <g_balance> <r_balance>
general_load_balance_test1()
{
	local g_cpus="$1"
	local g_balance="$2"
	local r_balance="$3"
	local g_path="$CPUSET/1"

	local fifo=
	local ret=

	cpuset_log "general group load balance test"
	cpuset_log "root group info:"
	cpuset_log "     sched load balance:" $r_balance
	cpuset_log "general group info:"
	cpuset_log "     cpus:" $g_cpus
	cpuset_log "     sched load balance:" $g_balance

	cpuset_set "$CPUSET" "-" "$mems_all" "$r_balance" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set root group parameter failed."
		return 1
	fi

	cpuset_set "$g_path" "$g_cpus" "$mems_all" "$g_balance" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group parameter failed."
		return 1
	fi

	cpuset_cpu_hog 2> $CPUSET_TMP/cpu-hog_stderr &
	pid=$!

	read fifo < ./myfifo
	if [ $fifo -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "There is something wrong with test tasks"
		return 1
	fi

	if [ "$g_cpus" != "-" ]; then
		cpuset_set_opt "$g_path" "tasks" "$pid" 2> $CPUSET_TMP/stderr
		if [ $? -ne 0 ]; then
			cpuset_log_error $CPUSET_TMP/stderr
			tst_resm TFAIL "attach test tasks to group "\
						"failed."
			/bin/kill -s SIGKILL $pid
			return 1
		fi
	fi

	# start to fork the child processes
	/bin/kill -s SIGUSR1 $pid 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "send the signal to fork the child tasks " \
			   "failed."
		/bin/kill -s SIGKILL $pid
		return 1
	fi

	read fifo < ./myfifo
	if [ $fifo -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/cpu-hog_stderr
		tst_resm TFAIL "forking test tasks failed"
		return 1
	fi

	# start to run the child processes
	/bin/kill -s SIGUSR1 $pid 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "send the signal to run the child tasks " \
			   "failed."
		/bin/kill -s SIGUSR2 $pid
		return 1
	fi

	sleep $runtime
	/bin/kill -s SIGUSR2 $pid 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "send the signal to stop the child tasks " \
			   "failed."
		return 1
	fi

	wait $pid
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/cpu-hog_stderr
		tst_resm TFAIL "load balance test failed."
		return 1
	fi

	tst_resm TPASS "load balance test succeeded."
}

# general_load_balance_test2 <g1_cpus> <g1_balance>
#			     <g2_cpus> <g2_balance>
#			     <cpuoffline>
general_load_balance_test2()
{
	local g1_cpus=$1
	local g1_balance=$2
	local g1_path="$CPUSET/1"

	local g2_cpus=$3
	local g2_balance=$4
	local g2_path="$CPUSET/2"

	local cpuoffline=$5

	local pid=
	local ret=

	cpuset_log "general group load balance test"
	cpuset_log "root group info:"
	cpuset_log "     sched load balance:" 0
	cpuset_log "general group1 info:"
	cpuset_log "     cpus:" $g1_cpus
	cpuset_log "     sched load balance:" $g1_balance
	cpuset_log "general group2 info:"
	cpuset_log "     cpus:" $g2_cpus
	cpuset_log "     sched load balance:" $g2_balance
	cpuset_log "CPU hotplug:" $cpuoffline

	cpuset_set "$CPUSET" "-" "$mems_all" "0" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set root group parameter failed."
		return 1
	fi

	cpuset_set "$g1_path" "$g1_cpus" "$mems_all" "$g1_balance" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group1 parameter failed."
		return 1
	fi

	cpuset_set "$g2_path" "$g2_cpus" "$mems_all" "$g2_balance" 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "set general group2 parameter failed."
		return 1
	fi

	# setup test environment
	setup_test_environment $cpuoffline 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "setup test environment(offline CPU#$HOTPLUG_CPU) failed"
		return 1
	fi

	cpuset_cpu_hog 2> $CPUSET_TMP/cpu-hog_stderr &
	pid=$!

	# wait for the parent to do prepare
	read fifo < ./myfifo
	if [ $fifo -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "There is something wrong with test tasks"
		return 1
	fi

	if [ "$g1_cpus" != "-" ]; then
		cpuset_set_opt "$g1_path" "tasks" "$pid" 2> $CPUSET_TMP/stderr
		if [ $? -ne 0 ]; then
			cpuset_log_error $CPUSET_TMP/stderr
			tst_resm TFAIL "attach test tasks to group "\
						"failed."
			/bin/kill -s SIGKILL $pid
			return 1
		fi
	fi

	# start to fork the child processes
	/bin/kill -s SIGUSR1 $pid 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "send the signal to fork the child tasks " \
			   "failed."
		/bin/kill -s SIGKILL $pid
		return 1
	fi

	# wait for the end of forking
	read fifo < ./myfifo
	if [ $fifo -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/cpu-hog_stderr
		tst_resm TFAIL "forking test tasks failed"
		return 1
	fi

	sleep 1
	cpu_hotplug $HOTPLUG_CPU $cpuoffline 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "$cpuoffline CPU#$HOTPLUG_CPU failed."
		/bin/kill -s SIGUSR2 $pid
		wait $pid
		return 1
	fi

	# start to run the child processes
	/bin/kill -s SIGUSR1 $pid 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "send the signal to run the child tasks " \
			   "failed."
		/bin/kill -s SIGUSR2 $pid
		wait $pid
		return 1
	fi

	sleep $runtime
	/bin/kill -s SIGUSR2 $pid 2> $CPUSET_TMP/stderr
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/stderr
		tst_resm TFAIL "send the signal to stop the child tasks " \
			   "failed."
		return 1
	fi

	wait $pid
	if [ $? -ne 0 ]; then
		cpuset_log_error $CPUSET_TMP/cpu-hog_stderr
		tst_resm TFAIL "load balance test failed."
		return 1
	fi

	tst_resm TPASS "load balance test succeeded."
}

base_test()
{
	setup
	if [ $? -ne 0 ]; then
		exit_status=1
	else
		$test_pro "$@"
		if [ $? -ne 0 ]; then
			exit_status=1
		fi

		cleanup
		if [ $? -ne 0 ]; then
			exit_status=1
		fi

		cpu_hotplug_cleanup
	fi
	TST_COUNT=$(($TST_COUNT + 1))
}

test_general_load_balance1()
{
	test_pro="general_load_balance_test1"

	local g_cpus=
	local g_isbalance=
	local r_isbalance=

	while read g_cpus g_isbalance r_isbalance
	do
		base_test $g_cpus $g_isbalance $r_isbalance
	done <<- EOF
		-	1	0
		1	0	0
		-	1	1
		1	1	1
		1,2	0	0
		1,2	1	0
		$cpus_all	1	0
	EOF
	# while read g_cpus g_isbalance r_isbalance
}

test_general_load_balance2()
{
	test_pro="general_load_balance_test2"

	local g1_cpus=
	local g1_isbalance=

	local g2_cpus=
	local g2_isbalance=

	local hotplug=

	while read g1_cpus g1_isbalance g2_cpus g2_isbalance hotplug
	do
		base_test $g1_cpus $g1_isbalance $g2_cpus $g2_isbalance $hotplug
	done <<- EOF
	1	1	0	1	none
	1,2	1	0-3	0	none
	1,2	1	0,3	1	none
	1,2	1	1,3	1	none
	1,2	1	1,3	1	offline
	1,2	1	1,3	1	online
	EOF
	# while read g1_cpus g1_isbalance g2_cpus g2_isbalance hotplug
}

mkfifo myfifo
test_general_load_balance1
test_general_load_balance2
rm -f myfifo

exit $exit_status
