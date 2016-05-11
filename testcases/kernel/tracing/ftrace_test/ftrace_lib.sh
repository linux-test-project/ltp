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

cd $LTPROOT/testcases/bin

export TPATH="$PWD"
export DEBUGFS_PATH="$PWD/debugfs"
export TRACING_PATH="$PWD/debugfs/tracing"
export FPATH="$TPATH/ftrace_function"
export RPATH="$TPATH/ftrace_regression"
export SPATH="$TPATH/ftrace_stress"

. test.sh

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
	restore_old_setting

	umount $DEBUGFS_PATH
	rmdir $DEBUGFS_PATH
}

clean_up_exit()
{
	clean_up
	exit 1
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

		# run the test for $test_interval secs
		if [ $elapsed -ge $test_interval ]; then
			break
		fi
	}
}

trap clean_up_exit INT

tst_require_root

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
