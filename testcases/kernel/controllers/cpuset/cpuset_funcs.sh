#!/bin/sh
# usage: . cpuset_funcs.sh
# functions for cpuset test

################################################################################
##                                                                            ##
## Copyright (c) 2009 FUJITSU LIMITED                                         ##
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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
## Author: Miao Xie <miaox@cn.fujitsu.com>                                    ##
##                                                                            ##
################################################################################

. test.sh

cpu_string="`cat /sys/devices/system/cpu/online`"
NR_CPUS=`tst_ncpus`

if [ -f "/sys/devices/system/node/has_high_memory" ]; then
	mem_string="`cat /sys/devices/system/node/has_high_memory`"
else
	mem_string="`cat /sys/devices/system/node/has_normal_memory`"
fi
N_NODES="`echo $mem_string | tr ',' ' '`"
count=0
final_node=0
for item in $N_NODES; do
	delta=1
	if [ "${item#*-*}" != "$item" ]; then
		delta=$((${item#*-*} - ${item%*-*} + 1))
	fi
	final_node=${item#*-*}
	count=$((count + $delta))
done
final_node=$((final_node + 1))
N_NODES=$count

final_cpu=0
N_CPUS="`echo $cpu_string | tr ',' ' '`"
for item in $N_CPUS; do
	final_cpu=${item#*-*}
done
final_cpu=$((final_cpu + 1))

CPUSET="/dev/cpuset"
CPUSET_TMP="/tmp/cpuset_tmp"
CLONE_CHILDREN="/dev/cpuset/cgroup.clone_children"
CHILDREN_VALUE="0"
HOTPLUG_CPU="1"
SCHED_LB="/dev/cpuset/cpuset.sched_load_balance"
SCHED_LB_VALUE="0"

cpuset_log()
{
	tst_resm TINFO "$*"
}

# cpuset_log_error <error_file>
cpuset_log_error()
{
	local error_message=

	while read error_message
	do
		cpuset_log "$error_message"
	done < "$1"
}

ncpus_check()
{
	if [ $NR_CPUS -lt $1 ]; then
		tst_brkm TCONF "The total of CPUs is less than $1"
	fi
	# check online cpus whether match 0-num
	if [ $final_cpu -eq $NR_CPUS ]; then
		tst_resm TINFO "CPUs are numbered continuously starting at 0 ($cpu_string)"
	else
		tst_brkm TCONF "CPUs are not numbered continuously starting at 0 ($cpu_string)"
	fi
}

nnodes_check()
{
	if [ $N_NODES -lt $1 ]; then
		tst_brkm TCONF "The total of nodes is less than $1"
	fi
	# check online nodes whether match 0-num
	if [ $final_node -eq $N_NODES ]; then
		tst_resm TINFO "Nodes are numbered continuously starting at 0 ($mem_string)"
	else
		tst_brkm TCONF "Nodes are not numbered continuously starting at 0 ($mem_string)"
	fi
}

user_check()
{
	if [ $(id -u) != 0 ]; then
		tst_brkm TCONF "Test must be run as root"
	fi
}

cpuset_check()
{
        if [ -f /proc/cgroups ]; then
		CPUSET_CONTROLLER=`grep -w cpuset /proc/cgroups | cut -f1`
		CPUSET_CONTROLLER_VALUE=`grep -w cpuset /proc/cgroups | cut -f4`

		if [ "$CPUSET_CONTROLLER" = "cpuset" ] && [ "$CPUSET_CONTROLLER_VALUE" = "1" ]
		then
			return 0
		fi
	fi

	tst_brkm TCONF "Cpuset is not supported"
}

machine_check()
{
	if tst_virt_hyperv; then
		tst_brkm TCONF "Microsoft Hyper-V detected, no support for CPU hotplug"
	fi
}

# optional parameters (pass both or none of them):
# $1 - required number of cpus (default 2)
# $2 - required number of memory nodes (default 2)
check()
{
	user_check

	cpuset_check

	ncpus_check ${1:-2}

	nnodes_check ${2:-2}

	machine_check
}

# Create /dev/cpuset & mount the cgroup file system with cpuset
# clean any group created eralier (if any)
setup()
{
	if [ -e "$CPUSET" ]
	then
		tst_resm TWARN "$CPUSET already exist.. overwriting"
		cleanup || tst_brkm TFAIL "Can't cleanup... Exiting"
	fi

	mkdir -p "$CPUSET_TMP"
	mkdir "$CPUSET"
	mount -t cgroup -o cpuset cpuset "$CPUSET" 2> /dev/null
	if [ $? -ne 0 ]; then
		cleanup
		tst_brkm TFAIL "Could not mount cgroup filesystem with"\
					" cpuset on $CPUSET..Exiting test"
	fi

	CHILDREN_VALUE="`cat $CLONE_CHILDREN`"
	SCHED_LB_VALUE="`cat $SCHED_LB`"
}

# Write the cleanup function
cleanup()
{
	grep "$CPUSET" /proc/mounts >/dev/null 2>&1 || {
		rm -rf "$CPUSET" >/dev/null 2>&1
		return 0
	}

	echo $CHILDREN_VALUE > $CLONE_CHILDREN
	echo $SCHED_LB_VALUE > $SCHED_LB

	find "$CPUSET" -type d | sort | sed -n '2,$p' | tac | while read subdir
	do
		while read pid
		do
			/bin/kill -9 $pid > /dev/null 2>&1
			if [ $? -ne 0 ]; then
				tst_brkm TFAIL "Couldn't kill task - "\
							"$pid in the cpuset"
			fi
		done < "$subdir/tasks"
		rmdir "$subdir"
		if [ $? -ne 0 ]; then
			tst_brkm TFAIL "Couldn't remove subdir - "
						"$subdir in the cpuset"
		fi
	done

	umount "$CPUSET"
	if [ $? -ne 0 ]; then
		tst_brkm TFAIL "Couldn't umount cgroup filesystem with"\
					" cpuset on $CPUSET..Exiting test"
	fi
	rmdir "$CPUSET" > /dev/null 2>&1
	rm -rf "$CPUSET_TMP" > /dev/null 2>&1
}

# set the cpuset's parameter
# cpuset_set <cpusetpath> <cpus> <mems> <load_balance>
cpuset_set()
{
	local path="$1"
	mkdir -p "$path"
	if [ $? -ne 0 ]; then
		return 1
	fi

	local cpus="$2"
	local mems="$3"
	local load_balance="$4"

	if [ "$path" != "$CPUSET" ]; then
		if [ "$cpus" != "-" ]; then
			/bin/echo $cpus > $path/cpuset.cpus
			if [ $? -ne 0 ]; then
				return 1
			fi
		fi

		/bin/echo $mems > $path/cpuset.mems
		if [ $? -ne 0 ]; then
			return 1
		fi
	fi

	/bin/echo $load_balance > $path/cpuset.sched_load_balance
	if [ $? -ne 0 ]; then
		return 1
	fi
}

# cpu_hotplug cpu_id offline/online
cpu_hotplug()
{
	if [ "$2" = "online" ]; then
		/bin/echo 1 > "/sys/devices/system/cpu/cpu$1/online"
		if [ $? -ne 0 ]; then
			return 1
		fi
	elif [ "$2" = "offline" ]; then
		/bin/echo 0 > "/sys/devices/system/cpu/cpu$1/online"
		if [ $? -ne 0 ]; then
			return 1
		fi
	fi
}

# setup_test_environment <online | offline>
#   online  - online a CPU in testing, so we must offline a CPU first
#   offline - offline a CPU in testing, we needn't do anything
setup_test_environment()
{
	if [ "$1" = "online" ]; then
		cpu_hotplug $HOTPLUG_CPU offline
		if [ $? -ne 0 ]; then
			return 1
		fi
	fi
}

cpu_hotplug_cleanup()
{
	local cpus_array="$(seq -s' ' 1 $((NR_CPUS-1)))"
	local cpuid=
	for cpuid in $cpus_array
	do
		local file="/sys/devices/system/cpu/cpu$cpuid/online"
		local offline="$(cat $file)"
		if [ $offline -eq 0 ]; then
			cpu_hotplug $cpuid "online"
		fi
	done
}

