#!/bin/bash

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
## along with this program;  if not, write to the Free Software Foundation,   ##
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           ##
##                                                                            ##
## Author: Shi Weihua <shiwh@cn.fujitsu.com>                                  ##
##                                                                            ##
################################################################################

caseno=$1
subsystem=$2
pid=0;
release_agent_para=1;
release_agent_echo=1;
subsystem_str="debug";
remount_use_str="";
noprefix_use_str="";
release_agent_para_str="";
mounted=1

# not output debug info when stress test
no_debug=0

usage()
{
	echo "usage of cgroup_fj_function2.sh: "
	echo "  ./cgroup_fj_function2.sh -case number[1-13] -subsystem"
	echo "example: ./cgroup_fj_function2.sh 1 cpuset"
	echo "  will test the 1st case with cpuset"
}

exit_parameter()
{
	echo "ERROR: Wrong input parametes... Exiting test"
	exit -1;
}

export TESTROOT=`pwd`
export TMPFILE=$TESTROOT/tmp_tasks

. $TESTROOT/cgroup_fj_utility.sh

case1()
{
	do_mkdir 1 1 $mount_point/ltp_subgroup_2

	do_echo 1 0 $pid $mount_point/ltp_subgroup_1/tasks
	sleep 1
	do_echo 1 0 $pid $mount_point/ltp_subgroup_2/tasks
	sleep 1
	do_echo 1 1 $pid $mount_point/tasks
}

case2()
{
	do_mkdir 1 1 $mount_point/ltp_subgroup_2

	$TESTROOT/cgroup_fj_proc &
	pid2=$!
	sleep 1

	cat $mount_point/tasks > $TMPFILE
	nlines=`cat $TMPFILE | wc -l`
	for i in `seq 1 $nlines`
	do
		cur_pid=`sed -n "$i""p" $TMPFILE`
		if [ -e /proc/$cur_pid/ ];then
			do_echo 1 0 "$cur_pid" $mount_point/ltp_subgroup_1/tasks
		fi
	done

	sleep 1

	cat $mount_point/ltp_subgroup_1/tasks > $TMPFILE
	nlines=`cat $TMPFILE | wc -l`
	for i in `seq 1 $nlines`
	do
		cur_pid=`sed -n "$i""p" $TMPFILE`
		if [ -e /proc/$cur_pid/ ];then
			do_echo 1 0 "$cur_pid" $mount_point/ltp_subgroup_2/tasks
		fi
	done

	sleep 1

	cat $mount_point/ltp_subgroup_2/tasks > $TMPFILE
	nlines=`cat $TMPFILE | wc -l`
	for i in `seq 1 $nlines`
	do
		cur_pid=`sed -n "$i""p" $TMPFILE`
		if [ -e /proc/$cur_pid/ ];then
			do_echo 1 1 "$cur_pid" $mount_point/tasks
		fi
	done
}

case3()
{
	do_mkdir 0 1 $mount_point/ltp_subgroup_2

	do_mv 0 1 $mount_point/ltp_subgroup_1 $mount_point/ltp_subgroup_3
}

case4()
{
	do_mkdir 0 1 $mount_point/ltp_subgroup_2

	do_mv 0 0 $mount_point/ltp_subgroup_1 $mount_point/ltp_subgroup_2
}

case5()
{
	mount_str="`mount -l | grep /dev/cgroup2`"
	if [ "$mount_str" != "" ]; then
		do_umount 1 1 /dev/cgroup2
	fi

	if [ -e /dev/cgroup2 ]; then
		do_rmdir 0 1 /dev/cgroup2
	fi

	do_mkdir 0 1 /dev/cgroup2

	do_mkdir 0 1 /dev/cgroup2/ltp_subgroup_2

	do_mv 0 1 $mount_point/ltp_subgroup_1 $mount_point/ltp_subgroup_2

	sleep 1

	do_rmdir 0 1 /dev/cgroup2/ltp_subgroup_2
	do_rmdir 0 1 /dev/cgroup2
}

case6()
{
	do_mkdir 0 1 $mount_point/ltp_subgroup_2

	do_mv 0 0 $mount_point/ltp_subgroup_1 $mount_point/tasks
}

case7()
{
	do_echo 0 1 $pid $mount_point/ltp_subgroup_1/tasks

	sleep 1

	do_rmdir 0 0 $mount_point/ltp_subgroup_1

	sleep 1

	do_echo 1 1 $pid $mount_point/tasks
}

case8()
{
	do_mkdir 0 1 $mount_point/ltp_subgroup_1/ltp_subgroup_1_1

	sleep 1

	do_rmdir 0 0 $mount_point/ltp_subgroup_1

	do_rmdir 1 1 $mount_point/ltp_subgroup_1/ltp_subgroup_1_1
}

case9()
{
	do_rmdir 0 1 $mount_point/ltp_subgroup_1
}

##########################  main   #######################
if [ "$#" -ne "2" ] || [ $caseno -lt 1 ] || [ $caseno -gt 9 ]; then
	usage;
	exit_parameter;
fi

exist_subsystem $subsystem

setup;

$TESTROOT/cgroup_fj_proc &
pid=$!
mkdir_subgroup;

case$caseno

cleanup;
do_kill 1 1 9 $pid
sleep 1
exit 0;
