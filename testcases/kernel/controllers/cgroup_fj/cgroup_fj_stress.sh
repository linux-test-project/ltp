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

subsystem=$1			# debug, cpuset, ns, cpu, cpuacct, memory...
subgroup_num=$2			#subgroup number in the same hierarchy
				#1: 1
				#2: 100
subgroup_hiers=$3		#number of subgroup's hierarchy
				#1: 1
				#2: 100
attach_operation=$4		# 1: attach one process to every subcgroup
				# 2: attach all processes in root group to one subcgroup
				# 3: attach all processes in root group to every subcgroup
mounted=1

usage()
{
	echo "usage of cgroup_fj_stress.sh: "
	echo "  ./cgroup_fj_stress.sh -subsystem -subgroup_num -subgroup_hiers -attach_operation"
	echo "    subsystem's usable number"
	echo "      debug"
	echo "      cpuset"
	echo "      ns"
	echo "      cpu"
	echo "      cpuacct"
	echo "      memory"
	echo "      freezer"
	echo "      devices"
	echo "      blkio"
	echo "      hugetlb"
	echo "      net_cls"
	echo "      net_prio"
	echo "      pids"
	echo "    subgroup_num's usable number"
	echo "      (subgroup number in the same hierarchy)"
	echo "      1"
	echo "      100"
        echo "    subgroup_hiers's usable number"
	echo "      (number of subgroup's hierarchy)"
	echo "      1"
	echo "      100"
	echo "    attach_operation's usable number"
	echo "      1: attach one process to every subcgroup"
	echo "      2: attach all processes in root group to one subcgroup"
	echo "      3: attach all processes in root group to every subcgroup"
        echo "example: ./cgroup_fj_stress.sh debug 1 1 1 1"
        echo "  will use "debug" to test, will mount once, will create one subgroup in same hierarchy,"
	echo "  will create one hierarchy, will attach one process to every subcgroup"
}

exit_parameter()
{
	echo "ERROR: Wrong input parameters... Exiting test"
	exit -1;
}

export TMPFILE=$TMPDIR/tmp_tasks.$$

. cgroup_fj_utility.sh

pid=0;
release_agent_para=1;
release_agent_echo=1;
subsystem_str=$subsystem;
if [ "$?" -ne "0" ] || [ "$#" -ne "4" ]; then
	usage;
	exit_parameter;
fi
remount_use_str="";
noprefix_use_str="";
release_agent_para_str="";
ulimit_u=`ulimit -u`
no_debug=1
cur_subgroup_path1=""
cur_subgroup_path2=""

get_subgroup_path1()
{
	cur_subgroup_path1=""
	if [ "$#" -ne 1 ] || [ "$1" -lt 1 ] || [ "$1" -gt $ulimit_u ]; then
		return;
	fi

	cur_subgroup_path1="$mount_point/ltp_subgroup_$1/"
}

get_subgroup_path2()
{
	cur_subgroup_path2=""
	if [ "$#" -ne 1 ] || [ "$1" -lt 2 ] || [ "$1" -gt $ulimit_u ]; then
		return;
	fi

	for i in `seq 2 $1`
	do
		cur_subgroup_path2="$cur_subgroup_path2""s/"
	done
}

case $subgroup_num in
''|*[!0-9]*)
	usage
	exit_parameter;;
    *) ;;
esac

case $subgroup_hiers in
''|*[!0-9]*)
	usage
	exit_parameter;;
    *) ;;
esac

##########################  main   #######################
exist_subsystem;
setup;

cgroup_fj_proc &
pid=$!

cpus=0
mems=0
exist_cpuset=0
exist_cpuset=`grep -w cpuset /proc/cgroups | cut -f1`;
if [ "$subsystem" == "cpuset" ]; then
	if [ "$exist_cpuset" != "" ]; then
		cpus=`cat $mount_point/cpuset.cpus`
		mems=`cat $mount_point/cpuset.mems`
	fi
fi

mkdir_subgroup;

# cpuset.cpus and cpuset.mems should be specified with suitable value
# before attachint operation if subsystem is cpuset
if [ "$subsystem" == "cpuset" ]; then
	if [ "$exist_cpuset" != "" ]; then
		do_echo 1 1 "$cpus" $mount_point/ltp_subgroup_1/cpuset.cpus;
		do_echo 1 1 "$mems" $mount_point/ltp_subgroup_1/cpuset.mems;
	fi
fi

get_subgroup_path2 $subgroup_hiers
count=0
pathes[1]=""
for i in `seq 1 $subgroup_num`
do
	get_subgroup_path1 $i
	do_mkdir 1 1 $cur_subgroup_path1
	if [ "$subsystem" == "cpuset" ]; then
		if [ "$exist_cpuset" != "" ]; then
			do_echo 1 1 "$cpus" "$cur_subgroup_path1""cpuset.cpus";
			do_echo 1 1 "$mems" "$cur_subgroup_path1""cpuset.mems";
		fi
	fi
	let "count = $count + 1"
	pathes[$count]="$cur_subgroup_path1"
	for j in `seq 2 $subgroup_hiers`
	do
		get_subgroup_path2 $j
		do_mkdir 1 1 "$cur_subgroup_path1""$cur_subgroup_path2" 1
		if [ "$subsystem" == "cpuset" ]; then
			if [ "$exist_cpuset" != "" ]; then
				do_echo 1 1 "$cpus" "$cur_subgroup_path1""$cur_subgroup_path2""cpuset.cpus";
				do_echo 1 1 "$mems" "$cur_subgroup_path1""$cur_subgroup_path2""cpuset.mems";
			fi
		fi
		let "count = $count + 1"
		pathes[$count]="$cur_subgroup_path1""$cur_subgroup_path2"
	done
done
echo "...mkdired $count times"

tst_sleep 100ms

case $attach_operation in
"1" )
	for i in `seq 1 $count`
	do
		do_echo 1 1 $pid "${pathes[$i]}""tasks"
	done
	do_echo 1 1 $pid $mount_point/tasks
	;;
"2" )
	pathes2[0]="$mount_point"
	pathes2[1]="${pathes[$count]}"
	pathes2[3]="$mount_point/"
	for i in `seq 1 $nlines`
	do
		j=$i
		let "j = $j + 1"
		cat "${pathes2[$i]}tasks" > $TMPFILE
		nlines=`cat "$TMPFILE" | wc -l`
		if [ $no_debug -ne 1 ]; then
			echo "DEBUG: move $nlines processes from "$i"th path to "$j"th"
		fi
		for k in `seq 1 $nlines`
		do
			cur_pid=`sed -n "$k""p" $TMPFILE`
			if [ -e /proc/$cur_pid/ ];then
				do_echo 0 1 "$cur_pid" "${pathes[$j]}tasks"
			fi
		done
	done
	;;
"3" )
	count2=$count
	let "count2 = $count2 + 1"
	pathes[0]="$mount_point/"
	pathes[$count2]="$mount_point/"
	for i in `seq 0 $count`
	do
		j=$i
		let "j = $j + 1"
		cat "${pathes[$i]}tasks" > $TMPFILE
		nlines=`cat "$TMPFILE" | wc -l`
		if [ $no_debug -ne 1 ]; then
			echo "DEBUG: move $nlines processes from "$i"th path to "$j"th"
		fi
		for k in `seq 1 $nlines`
		do
			cur_pid=`sed -n "$k""p" $TMPFILE`
			if [ -e /proc/$cur_pid/ ];then
				do_echo 0 1 "$cur_pid" "${pathes[$j]}tasks"
			fi
		done
	done
	;;
*  )
	;;
esac
reclaim_foundling;
for i in `seq 1 $count`
do
	j=i
	let "j = $count - $j + 1"
	do_rmdir 1 1 ${pathes[$j]}
done

do_rmdir 0 1 $mount_point/ltp_subgroup_*

tst_sleep 100ms

cleanup;
do_kill 1 1 9 $pid
tst_sleep 100ms
exit 0;
