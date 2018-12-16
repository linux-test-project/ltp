#!/bin/sh

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

. test.sh

ftrace_test_init()
{
	export TPATH="$PWD"
	export SPATH="$TPATH/ftrace_stress"

	if grep -q debugfs /proc/mounts; then
		export DEBUGFS_PATH=/sys/kernel/debug/
		export TRACING_PATH="$DEBUGFS_PATH/tracing"
		debugfs_def_mounted=1
	else
		tst_tmpdir
		export DEBUGFS_PATH="$PWD/debugfs"
		export TRACING_PATH="$PWD/debugfs/tracing"
		mkdir $DEBUGFS_PATH
		mount -t debugfs xxx $DEBUGFS_PATH
	fi

	TST_CLEANUP=clean_up

	trap clean_up_exit INT

	tst_require_root

	# Check to see tracing feature is supported or not
	if [ ! -d $TRACING_PATH ]; then
		tst_brkm TCONF "Tracing is not supported. Skip the test..."
	fi

	save_old_setting
}

test_interval=$1

save_old_setting()
{
	cd $TRACING_PATH

	old_trace_options=( `cat trace_options` )
	old_tracing_on=`cat tracing_on`
	old_buffer_size=`cat buffer_size_kb`
	old_tracing_cpumask=`cat tracing_cpumask`

	if [ -e tracing_cpumask ]; then
		old_tracing_cpumask=`cat tracing_cpumask`
	fi

	if [ -e tracing_enabled ]; then
		old_tracing_enabled=`cat tracing_enabled`
	fi

	if [ -e stack_max_size ]; then
		old_stack_tracer_enabled=`cat /proc/sys/kernel/stack_tracer_enabled`
	fi

	if [ -e "/proc/sys/kernel/ftrace_enabled" ]; then
		old_ftrace_enabled=`cat /proc/sys/kernel/ftrace_enabled`
	fi

	if [ -e "function_profile_enabled" ]; then
		old_profile_enabled=`cat function_profile_enabled`
	fi

	setting_saved=1

	cd - > /dev/null
}

restore_old_setting()
{
	if [ ! "$setting_saved" = 1 ]; then
		return
	fi

	cd $TRACING_PATH

	echo nop > current_tracer
	echo 0 > events/enable
	if [ -e tracing_max_latency ]; then
		echo 0 > tracing_max_latency
	fi

	if [ -e tracing_cpumask ]; then
		echo $old_tracing_cpumask > tracing_cpumask
	fi

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

	if [ -e tracing_enabled ];then
		echo $old_tracing_enabled > tracing_enabled
	fi

	for option in $old_trace_options
	do
		echo $option > trace_options 2> /dev/null
	done

	echo > trace

	if [ -f set_ftrace_filter ]; then
		echo  > set_ftrace_filter
	fi

	cd - > /dev/null
}

clean_up_mount()
{
	if [ ! "$debugfs_def_mounted" = "1" ]; then
		umount $DEBUGFS_PATH
		rmdir $DEBUGFS_PATH
	fi
}

clean_up()
{
	restore_old_setting
	clean_up_mount
}

clean_up_exit()
{
	restore_old_setting
	clean_up_mount
	exit 1
}

test_begin()
{
	start_time=`date +%s`
}

test_wait()
{
	# run the test for $test_interval secs
	tst_sleep ${test_interval}s
}

ftrace_test_init
