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
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
## Author: Miao Xie <miaox@cn.fujitsu.com>                                    ##
##                                                                            ##
################################################################################

NR_CPUS="`cat /proc/cpuinfo | grep "processor" | wc -l`"
if [ -f "/sys/devices/system/node/has_high_memory" ]; then
	N_NODES="`cat /sys/devices/system/node/has_high_memory`"
else
	N_NODES="`cat /sys/devices/system/node/has_normal_memory`"
fi
N_NODES=${N_NODES#*-*}
((N_NODES++))

CPUSET="/dev/cpuset"
CPUSET_TMP="/tmp/cpuset_tmp"

cpuset_log()
{
	tst_resm TINFO "$*"
}

version_check()
{
	tst_kvercmp 2 6 28
	if [ $? -eq 0 ]; then
		tst_brkm TCONF ignored "kernel is below 2.6.28"
		return 1
	fi
}

ncpus_check()
{
	if [ $NR_CPUS -lt 4 ]; then
		tst_brkm TCONF ignored "The total of CPUs is less than 4"
		return 1
	fi
}

nnodes_check()
{
	if [ $N_NODES -lt 3 ]; then
		tst_brkm TCONF ignored "The total of nodes is less than 3"
		return 1
	fi
}

user_check()
{
	if [ "$USER" != root ]; then
		tst_brkm TCONF ignored "Test must be run as root"
		return 1
	fi
}

cpuset_check()
{
	grep cpuset /proc/cgroups &> /dev/null
	if [ $? -ne 0 ]; then
		tst_brkm TCONF ignored "Cpuset is not supported"
		return 1
	fi
}

check()
{
	user_check
	if [ $? -ne 0 ]; then
		return 1
	fi

	cpuset_check
	if [ $? -ne 0 ]; then
		return 1
	fi

	version_check
	if [ $? -ne 0 ]; then
		return 1
	fi

	ncpus_check
	if [ $? -ne 0 ]; then
		return 1
	fi

	nnodes_check
	if [ $? -ne 0 ]; then
		return 1
	fi

}

# Create /dev/cpuset & mount the cgroup file system with cpuset
# clean any group created eralier (if any)
setup()
{
	if [ -e "$CPUSET" ]
	then
		tst_resm TWARN "$CPUSET already exist.. overwriting"
		cleanup || {
			tst_brkm TFAIL ignored "Can't cleanup... Exiting"
			return 1
		}
	fi

	mkdir -p "$CPUSET_TMP"
	mkdir "$CPUSET"
	mount -t cpuset cpuset "$CPUSET" 2> /dev/null
	if [ $? -ne 0 ]; then
		tst_brkm TFAIL ignored "Could not mount cgroup filesystem with"\
					" cpuset on $CPUSET..Exiting test"
		cleanup
		return 1
	fi
}

# Write the cleanup function
cleanup()
{
	mount | grep "$CPUSET" &>/dev/null || {
		rm -rf "$CPUSET" &>/dev/null
		return 0
	}

	find "$CPUSET" -type d | sort | sed -n '2,$p' | tac | while read subdir
	do
		while read pid
		do
			/bin/kill $pid &> /dev/null
			if [ $? -ne 0 ]; then
				tst_brkm TFAIL ignored "Couldn't kill task - "\
							"$pid in the cpuset"
				return 1
			fi
		done < "$subdir/tasks"
		rmdir "$subdir"
		if [ $? -ne 0 ]; then
			tst_brkm TFAIL ignored "Couldn't remove subdir - "
						"$subdir in the cpuset"
			return 1
		fi
	done

	umount "$CPUSET"
	if [ $? -ne 0 ]; then
		tst_brkm TFAIL ignored "Couldn't umount cgroup filesystem with"\
					" cpuset on $CPUSET..Exiting test"
		return 1
	fi
	rmdir "$CPUSET" &> /dev/null
	rm -rf "$CPUSET_TMP" &> /dev/null
}

# set_cfiles_value <path> <value>
set_cfiles_value()
{
	local path=$1
	local value=$2

	echo $value > $path
	return $?
}
