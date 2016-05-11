#! /bin/sh

###########################################################################
##                                                                       ##
## Copyright (c) 2010 FUJITSU LIMITED                                    ##
##                                                                       ##
## This program is free software: you can redistribute it and/or modify  ##
## it under the terms of the GNU General Public License as published by  ##
## the Free Software Foundation, either version 3 of the License, or     ##
## (at your option) any later version.                                   ##
##                                                                       ##
## This program is distributed in the hope that it will be useful,       ##
## but WITHOUT ANY WARRANTY; without even the implied warranty of        ##
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          ##
## GNU General Public License for more details.                          ##
##                                                                       ##
## You should have received a copy of the GNU General Public License     ##
## along with this program. If not, see <http://www.gnu.org/licenses/>.  ##
##                                                                       ##
## Author: Li Zefan <lizf@cn.fujitsu.com>                                ##
##                                                                       ##
###########################################################################

export TCID="ftrace-stress-test"
export TST_TOTAL=1
export TST_COUNT=1

. ftrace_lib.sh

test_targets=" \
trace_pipe current_tracer ftrace_enabled function_profile_enabled \
set_event set_ftrace_pid stack_max_size stack_trace trace trace_clock \
trace_options trace_stat tracing_enabled tracing_max_latency \
tracing_on function_profile_enabled buffer_size_kb tracing_cpumask \
set_ftrace_filter"

get_skip_targets()
{
	NR_PIDS=0
	for target in ${test_targets}; do
		if [ ! -e $TRACING_PATH/$target ] &&
			[ ! -e /proc/sys/kernel/$target ]; then
			eval skip_$target=1
			tst_resm TINFO "$target is not supported. Skip it."
		else
			eval skip_$target=0
			NR_PIDS=$((NR_PIDS + 1))
		fi
	done
	# Export it before sub case is lanuched.
	export NR_PIDS
}

should_skip_target()
{
	local skip_var=skip_$1
	eval local skip_val=\$${skip_var}
	[ "$skip_val" = 1 ]
}

test_kill()
{
	tst_resm TINFO "killing ${pid0}"
	kill -USR1 ${pid0}
	wait ${pid0}

	local p=1;
	while [ $p -lt $NR_PIDS ]; do
		local pid_var=pid${p}
		eval local kill_pid=\$${pid_var}
		tst_resm TINFO "killing ${kill_pid}"
		kill -KILL $kill_pid
		wait ${kill_pid}
		p=$((p + 1))
	done
}

test_stress()
{
	local index=0;

	tst_resm TINFO "Test targets: ${test_targets}"

	get_skip_targets
	for target in ${test_targets}; do
		if should_skip_target $target; then
			continue
		fi
		sh ftrace_${target}.sh &
		eval pid${index}=$!
		tst_resm TINFO "Start pid${index}=$! $SPATH/ftrace_${target}.sh"
		index=$((index + 1))
	done
	export_pids
}

export_pids()
{
	local p=0
	while [ $p -lt $NR_PIDS ]; do
		export pid${p}
		p=$((p + 1))
	done
}

cd ftrace_stress/
# ----------------------------
tst_resm TINFO "Ftrace Stress Test Begin"

test_begin

test_stress

test_wait

test_kill

tst_resm TINFO "Finished running the test. Run dmesg to double-check for bugs"

tst_exit

