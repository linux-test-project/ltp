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

test_success=true

export_pids()
{
	export pid1 pid2 pid3 pid4 pid5 pid6 pid7 pid8 pid9 pid10 pid11 pid12 \
		pid13 pid14 pid15 pid16

	export NR_PIDS=16
}

test_stress()
{
	export_pids

	$SPATH/ftrace_trace_clock.sh &
	pid1=$!
	$SPATH/ftrace_current_tracer.sh &
	pid2=$!
	$SPATH/ftrace_trace_options.sh &
	pid3=$!
	$SPATH/ftrace_tracing_max_latency.sh &
	pid4=$!
	$SPATH/ftrace_stack_trace.sh &
	pid5=$!
	$SPATH/ftrace_stack_max_size.sh &
	pid6=$!
	$SPATH/ftrace_tracing_on.sh &
	pid7=$!
	$SPATH/ftrace_tracing_enabled.sh &
	pid8=$!
	$SPATH/ftrace_set_event.sh &
	pid9=$!
	$SPATH/ftrace_buffer_size.sh &
	pid10=$!
	$SPATH/ftrace_trace.sh &
	pid11=$!
	$SPATH/ftrace_trace_pipe.sh &
	pid12=$!
	$SPATH/ftrace_ftrace_enabled.sh &
	pid13=$!
	$SPATH/ftrace_set_ftrace_pid.sh &
	pid14=$!
	$SPATH/ftrace_profile_enabled.sh &
	pid15=$!
	$SPATH/ftrace_trace_stat.sh &
	pid16=$!
}

test_kill()
{
	kill -KILL $pid1 || test_success=false
	kill -KILL $pid2 || test_success=false
	kill -KILL $pid3 || test_success=false
	kill -KILL $pid4 || test_success=false
	kill -KILL $pid5 || test_success=false
	kill -KILL $pid6 || test_success=false
	kill -KILL $pid7 || test_success=false
	kill -KILL $pid8 || test_success=false
	kill -KILL $pid9 || test_success=false
	kill -KILL $pid10 || test_success=false
	kill -KILL $pid11 || test_success=false
	kill -USR1 $pid12 || test_success=false
	kill -KILL $pid13 || test_success=false
	kill -KILL $pid14 || test_success=false
	kill -KILL $pid15 || test_success=false
	kill -KILL $pid16 || test_success=false

	sleep 2
	clean_up
}


# ----------------------------
echo "Ftrace Stress Test Begin"

save_old_setting

test_begin

test_stress

test_wait

test_kill

echo "Ftrace Stress Test End"

if $test_success; then
	tst_resm TPASS "finished running the test. Run dmesg to double-check for bugs"
else
	tst_resm TFAIL "please check log message."
	exit 1
fi
