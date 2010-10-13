#! /bin/sh

################################################################################
##                                                                            ##
## Copyright (c) 2010 FUJITSU LIMITED                                         ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
## Author: Li Zefan <lizf@cn.fujitsu.com>                                     ##
##                                                                            ##
################################################################################

cd $LTPROOT/testcases/bin

export TCID="ftrace-stress-test"
export TST_TOTAL=1
export TST_COUNT=1

export TPATH="$PWD"
export DEBUGFS_PATH="$PWD/debugfs"
export TRACING_PATH="$PWD/debugfs/tracing"
export SPATH="$TPATH/ftrace_stress"

test_interval=$1

save_old_setting()
{
	cd $TRACING_PATH

	old_trace_options=( `cat trace_options` )
	old_tracing_on=`cat tracing_on`
	old_tracing_enabled=`cat tracing_enabled`
	old_buffer_size=`cat buffer_size_kb`

	if [ -e stack_max_size ]; then
		old_stack_tracer_enabled=`cat /proc/sys/kernel/stack_tracer_enabled`
	fi

	if [ -e "/proc/sys/kernel/ftrace_enabled" ]; then
		old_ftrace_enabled=`cat /proc/sys/kernel/ftrace_enabled`
	fi

	if [ -e "function_profile_enabled" ]; then
		old_profile_enabled=`cat function_profile_enabled`
	fi

	cd - > /dev/null
}

restore_old_setting()
{
	cd $TRACING_PATH

	echo nop > current_tracer
	echo 0 > events/enable
	echo 0 > tracing_max_latency 2> /dev/null

	if [ -e trace_clock ]; then
		echo local > trace_clock
	fi

	if [ -e "function_pofile_enabled" ]; then
		echo $old_profile_enabled > function_profile_enabled
	fi

	if [ -e "/proc/sys/kernel/ftrace_enabled" ]; then
		echo $old_ftrace_enabled > /proc/sys/kernel/ftrace_enabled
	fi

	if [ -e stack_max_size ]; then
		echo $old_stack_tracer_enabled > /proc/sys/kernel/stack_tracer_enabled
		echo 0 > stack_max_size
	fi

	echo $old_buffer_size > buffer_size_kb
	echo $old_tracing_on > tracing_on
	echo $old_tracing_enabled > tracing_enabled

	for option in $old_trace_options
	do
		echo $option > trace_options 2> /dev/null
	done

	echo > trace

	cd - > /dev/null
}

clean_up()
{
	kill -KILL $pid1
	kill -KILL $pid2
	kill -KILL $pid3
	kill -KILL $pid4
	kill -KILL $pid5
	kill -KILL $pid6
	kill -KILL $pid7
	kill -KILL $pid8
	kill -KILL $pid9
	kill -KILL $pid10
	kill -KILL $pid11
	kill -USR1 $pid12
	kill -KILL $pid13
	kill -KILL $pid14
	kill -KILL $pid15
	kill -KILL $pid16

	sleep 2
	restore_old_setting

	umount $DEBUGFS_PATH
	rmdir $DEBUGFS_PATH
}

clean_up_exit()
{
	clean_up
	exit 1
}

export_pids()
{
	export pid1 pid2 pid3 pid4 pid5 pid6 pid7 pid8 pid9 pid10 pid11 pid12 \
		pid13 pid14 pid15 pid16

	export NR_PIDS=16
}

test_begin()
{
	start_time=`date +%s`
}

test_wait()
{
	for ((; ;))
	{
		sleep 2

		cur_time=`date +%s`
		elapsed=$(( $cur_time - $start_time ))

		# run the stress test for $test_interval secs
		if [ $elapsed -ge $test_interval ]; then
			break
		fi
	}
}

trap clean_up_exit SIGINT

# Should be run by root user
if [ `id -ru` != 0 ]; then
	tst_brkm TCONF ignored "The test should be run by root user. Skip the test..."
	exit 0
fi

# Don't run the test on kernels older than 2.6.34, otherwise
# it can crash the system if the kernel is not latest-stable
tst_kvercmp 2 6 34
if [ $? -eq 0 ]; then
	tst_brkm TCONF ignored "The test should be run in kernels >= 2.6.34. Skip the test..."
	exit 0
fi

mkdir $DEBUGFS_PATH
mount -t debugfs xxx $DEBUGFS_PATH

# Check to see tracing feature is supported or not
if [ ! -d $TRACING_PATH ]; then
	tst_brkm TCONF ignored "Tracing is not supported. Skip the test..."
	umount $DEBUGFS_PATH
	rmdir $DEBUGFS_PATH
	exit 0
fi

echo "Ftrace Stress Test Begin"

save_old_setting

test_begin

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

export_pids

test_wait

clean_up

echo "Ftrace Stress Test End"

tst_resm TPASS "finished running the test. Run dmesg to double-check for bugs"

