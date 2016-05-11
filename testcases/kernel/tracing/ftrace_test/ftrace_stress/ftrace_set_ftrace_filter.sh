#! /bin/sh
###########################################################################
##                                                                       ##
## Copyright (c) 2015, Red Hat Inc.                                      ##
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
## Author: Chunyu Hu <chuhu@redhat.com>                                  ##
##                                                                       ##
###########################################################################

. test.sh

triggers="traceon traceoff enable_event disable_event snapshot \
	 dump cpudump stacktrace module function"
nr_triggers=$(echo ${triggers} | wc -w)

module_pick()
{
	nr_module=$(lsmod | wc -l)
	pick_one=$(tst_random 1 $nr_module)
	picked_module=$(lsmod | awk "{if (NR == $pick_one) {print \$1}}")
}

filter_file=$TRACING_PATH/available_filter_functions
nr_functions=$(awk 'END{print NR}' $filter_file)

function_pick()
{
	if [ -f $filter_file ]; then
		local pick_one=$(tst_random 1 $nr_functions)
		picked_function=$(awk "{if (NR == $pick_one) {print \$1}}" $filter_file)
		echo $picked_function
	else
		echo "\*sched\*"
	fi
}

event_pick()
{
	local events_file=$TRACING_PATH/available_events
	if [ -f $events_file ]; then
		nr_events=$(awk 'END{print NR}' $events_file)
		local pick_one=$(tst_random 1 $nr_events)
		picked_event=$(awk "{if (NR == $pick_one) {print \$0}}" $events_file)
		echo "$picked_event"
	else
		echo "sched:sched_switch"
	fi
}

filter_formatter()
{
	function_str=$(function_pick)
	count=$(tst_random 0 2)

	case $1 in
	traceon|traceoff|snapshot|dump|cpudump|stacktrace)
		trigger=$1
		;;
	enable_event|disable_event)
		event_sys_name=$(event_pick)
		trigger=$1:$event_sys_name
		;;
	module)
		module_pick
		echo ":mod:$picked_module"
		return
		;;
	function)
		echo "$function_str"
		return
		;;
	*)
		trigger=$1
		;;
	esac

	if [ $count -gt 0 ]; then
		trigger=$trigger:$count
	fi
	echo $function_str:$trigger
}

signal_handler()
{
	tst_exit
}

trap signal_handler SIGTERM

while true; do
	# Here try to check if a race caused issue can be hit.
	cat $TRACING_PATH/set_ftrace_filter > /dev/null

	trigger_index=$(tst_random 1 $nr_triggers)
	trigger_name=$(echo $triggers | awk "{print \$$trigger_index}")
	filter_format=$(filter_formatter $trigger_name)

	echo "$filter_format" > $TRACING_PATH/set_ftrace_filter
	[ $? -ne 0 ] && tst_resm TFAIL "$0: setup filter <$filter_format> failed"

	sleep 2

	echo "!$filter_format" > $TRACING_PATH/set_ftrace_filter
	[ $? -ne 0 ] && tst_resm TFAIL "$0: remove filter <$filter_format> failed"
done

