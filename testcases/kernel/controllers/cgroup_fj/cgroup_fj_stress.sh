#! /bin/sh

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
## Author: Shi Weihua <shiwh@cn.fujitsu.com>                                  ##
##                                                                            ##
################################################################################

subsystem=$1			# 1: debug
				# 2: cpuset
				# 3: ns
				# 4: cpu
				# 5: cpuacct
				# 6: memory
				# 7: all
mount_times=$2			#1: execute once
				#2: execute 100 times
subgroup_num=$3			#subgroup number in the same hierarchy
				#1: 1
				#2: 100
subgroup_hiers=$4		#number of subgroup's hierarchy
				#1: 1
				#2: 100
attach_operation=$5		# 1: attach one process to every subcgroup
				# 2: attach all processes in root group to one subcgroup
				# 3: attach all processes in root group to every subcgroup

usage()
{
	echo "usage of cgroup_fj_stress.sh: "
	echo "  ./cgroup_fj_stress.sh -subsystem -mount_times -subgroup_num -subgroup_hiers -attach_operation"
	echo "    subsystem's usable number"
	echo "      1: debug"
	echo "      2: cpuset"
	echo "      3: ns"
	echo "      4: cpu"
	echo "      5: cpuacct"
	echo "      6: memory"
	echo "      7: all"
	echo "    mount_times's usable number"
	echo "      1: execute once"
	echo "      2: execute 100 times"
	echo "    subgroup_num's usable number"
	echo "      (subgroup number in the same hierarchy)"
	echo "      1: 1"
	echo "      2: 100"
        echo "    subgroup_hiers's usable number"
	echo "      (number of subgroup's hierarchy)"
	echo "      1: 1"
	echo "      2: 100"
	echo "    attach_operation's usable number"
	echo "      1: attach one process to every subcgroup"
	echo "      2: attach all processes in root group to one subcgroup"
	echo "      3: attach all processes in root group to every subcgroup"
        echo "example: ./cgroup_fj_stress.sh 1 1 1 1 1"
        echo "  will use "debug" to test, will mount once, will create one subgroup in same hierarchy,"
	echo "  will create one hierarchy, will attach one process to every subcgroup"
}


exit_parameter()
{
	echo "ERROR: Wrong inputed parameter..Exiting test" >> $LOGFILE
	exit -1;
}

export TESTROOT=`pwd`
if [ "$LOGFILE" = "" ]; then
	LOGFILE="/dev/stdout"
fi
export TMPFILE=$TESTROOT/tmp_tasks

. $TESTROOT/cgroup_fj_utility.sh

pid=0;
release_agent_para=1;
release_agent_echo=1;
subsystem_str="debug";
get_subsystem;
if [ "$?" -ne "0" ] || [ "$#" -ne "5" ]; then
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
	
	cur_subgroup_path1="/dev/cgroup/subgroup_$1/"
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

case $mount_times in
"1" )
	mount_times=1
	;;
"2" )
	mount_times=100
	;;
*  )
	usage;
	exit_parameter;
	;;
esac

case $subgroup_num in
"1" )
	subgroup_num=1
	;;
"2" )
	subgroup_num=100
	;;
*  )
	usage;
	exit_parameter;
	;;
esac

case $subgroup_hiers in
"1" )
	subgroup_hiers=1
	;;
"2" )
	subgroup_hiers=100
	;;
*  )
	usage;
	exit_parameter;
	;;
esac

##########################  main   #######################
echo "-------------------------------------------------------------------------" >> $LOGFILE
echo "case no : $CASENO2" >> $LOGFILE
echo `date` >> $LOGFILE

setup;

echo "INFO: now we begin to stress test no $CASENO2 ..." >> $LOGFILE

mount_cgroup;

$TESTROOT/cgroup_fj_proc &
pid=$!

cpus=0
mems=0
exist_cpuset=0
exist_cpuset=`grep -w cpuset /proc/cgroups | cut -f1`;
if [ $subsystem -eq 2 ] || [ $subsystem -eq 7 ] ; then
	if [ "$exist_cpuset" != "" ]; then
		cpus=`cat /dev/cgroup/cpuset.cpus`
		mems=`cat /dev/cgroup/cpuset.mems`
	fi
fi

mkdir_subgroup;

# cpuset.cpus and cpuset.mems should be specified with suitable value 
# before attachint operation if subsystem is cpuset
if [ $subsystem -eq 2 ] || [ $subsystem -eq 7 ] ; then
	if [ "$exist_cpuset" != "" ]; then
		do_echo 1 1 "$cpus" /dev/cgroup/subgroup_1/cpuset.cpus;
		do_echo 1 1 "$mems" /dev/cgroup/subgroup_1/cpuset.mems;
	fi
fi

if [ $mount_times -ne 1 ]; then
	count=0
	for i in `seq 1 $mount_times`
	do
		do_echo 1 1 $pid /dev/cgroup/subgroup_1/tasks
		if [ $subsystem -eq 3 ] || [ $subsystem -eq 7 ] ; then
			do_kill 1 1 9 $pid
			$TESTROOT/cgroup_fj_proc &
			pid=$!
		else
			do_echo 1 1 $pid /dev/cgroup/tasks
		fi
		setup;
		$TESTROOT/cgroup_fj_proc &
		pid=$!
		mount_cgroup;
		mkdir_subgroup;
		if [ $subsystem -eq 2 ] || [ $subsystem -eq 7 ] ; then
			if [ "$exist_cpuset" != "" ]; then
				do_echo 1 1 "$cpus" /dev/cgroup/subgroup_1/cpuset.cpus;
				do_echo 1 1 "$mems" /dev/cgroup/subgroup_1/cpuset.mems;
			fi
		fi
		let "count = $count + 1"
		echo "$count .. OK" >> $LOGFILE
	done
	echo "...executed $count times" >> $LOGFILE
else
	get_subgroup_path2 $subgroup_hiers
	count=0
	pathes[1]=""
	for i in `seq 1 $subgroup_num`
	do
		get_subgroup_path1 $i
		do_mkdir 1 1 $cur_subgroup_path1
		if [ $subsystem -eq 2 ] || [ $subsystem -eq 7 ] ; then
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
			if [ $subsystem -eq 2 ] || [ $subsystem -eq 7 ] ; then
				if [ "$exist_cpuset" != "" ]; then
					do_echo 1 1 "$cpus" "$cur_subgroup_path1""$cur_subgroup_path2""cpuset.cpus";
					do_echo 1 1 "$mems" "$cur_subgroup_path1""$cur_subgroup_path2""cpuset.mems";
				fi
			fi
			let "count = $count + 1"
			pathes[$count]="$cur_subgroup_path1""$cur_subgroup_path2"
		done
	done
	echo "...mkdired $count times" >> $LOGFILE
	
	sleep 1
	
	case $attach_operation in
	"1" )
		for i in `seq 1 $count`
		do
			do_echo 1 1 $pid "${pathes[$i]}""tasks"
		done
		do_echo 1 1 $pid /dev/cgroup/tasks
		;;
	"2" )
		pathes2[0]="/dev/cgroup/"
		pathes2[1]="${pathes[$count]}"
		pathes2[3]="/dev/cgroup/"
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
		pathes[0]="/dev/cgroup/"
		pathes[$count2]="/dev/cgroup/"
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
fi

do_rmdir 0 1 /dev/cgroup/subgroup_*

sleep 1

cleanup;
do_kill 1 1 9 $pid
sleep 1
exit 0;
