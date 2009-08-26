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

caseno=$1
pid=0;
subsystem=1;
release_agent_para=1;
release_agent_echo=1;
subsystem_str="debug";
remount_use_str="";
noprefix_use_str="";
release_agent_para_str="";

# not output debug info when stress test
no_debug=0

usage()
{
	echo "usage of cgroup_fj_function2.sh: "
	echo "  ./cgroup_fj_function2.sh -case number[1-13]"
	echo "example: ./cgroup_fj_function2.sh 1"
	echo "  will test the 1st case"
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

case1()
{
	do_mkdir 1 1 /dev/cgroup/subgroup_2
	
	do_echo 1 0 $pid /dev/cgroup/subgroup_1/tasks
	sleep 1
	do_echo 1 0 $pid /dev/cgroup/subgroup_2/tasks
	sleep 1
	do_echo 1 1 $pid /dev/cgroup/tasks
}

case2()
{
	do_mkdir 1 1 /dev/cgroup/subgroup_2
	
	$TESTROOT/cgroup_fj_proc &
	pid2=$!
	sleep 1
	
	cat /dev/cgroup/tasks > $TMPFILE
	nlines=`cat $TMPFILE | wc -l`
	for i in `seq 1 $nlines`
	do
		cur_pid=`sed -n "$i""p" $TMPFILE`
		if [ -e /proc/$cur_pid/ ];then
			do_echo 1 0 "$cur_pid" /dev/cgroup/subgroup_1/tasks
		fi
	done
	
	sleep 1
	
	cat /dev/cgroup/subgroup_1/tasks > $TMPFILE
	nlines=`cat $TMPFILE | wc -l`
	for i in `seq 1 $nlines`
	do
		cur_pid=`sed -n "$i""p" $TMPFILE`
		if [ -e /proc/$cur_pid/ ];then
			do_echo 1 0 "$cur_pid" /dev/cgroup/subgroup_2/tasks
		fi
	done
	
	sleep 1
	
	cat /dev/cgroup/subgroup_2/tasks > $TMPFILE
	nlines=`cat $TMPFILE | wc -l`
	for i in `seq 1 $nlines`
	do
		cur_pid=`sed -n "$i""p" $TMPFILE`
		if [ -e /proc/$cur_pid/ ];then
			do_echo 1 1 "$cur_pid" /dev/cgroup/tasks
		fi
	done
}

case3()
{
	exist_subsystem "cpuset"
	exist_subsystem "ns"
	do_mount 1 1 "-odebug,cpuset,ns" /dev/cgroup cgroup1
	
	mount_str="`mount -l | grep /dev/cgroup2`"
	if [ "$mount_str" != "" ]; then
		do_umount 0 1 /dev/cgroup2
	fi
	
	if [ -e /dev/cgroup2 ]; then
		do_rmdir 1 1 /dev/cgroup2
	fi
	
	do_mkdir 1 1 /dev/cgroup2

	exist_subsystem "cpu"
	exist_subsystem "cpuacct"
	exist_subsystem "memory"
	do_mount 1 1 "-ocpu,cpuacct,memory" /dev/cgroup2 cgroup2
	
	sleep 1
	
	do_umount 0 1 /dev/cgroup
	do_rmdir 0 1 /dev/cgroup
	do_umount 0 1 /dev/cgroup2
	do_rmdir 0 1 /dev/cgroup2
}

case4()
{
	exist_subsystem "cpuset"
	exist_subsystem "ns"
	do_mount 1 1 "-odebug,cpuset,ns" /dev/cgroup cgroup1
	
	mount_str="`mount -l | grep /dev/cgroup2`"
	if [ "$mount_str" != "" ]; then
		do_umount 1 1 /dev/cgroup2
	fi
	
	if [ -e /dev/cgroup2 ]; then
		do_rmdir 0 1 /dev/cgroup2
	fi
	
	do_mkdir 0 1 /dev/cgroup2
	
	do_mount 1 1 "-odebug,cpuset,ns" /dev/cgroup2 cgroup2
	
	sleep 1
	
	do_umount 0 1 /dev/cgroup
	do_rmdir 0 1 /dev/cgroup
	do_umount 0 1 /dev/cgroup2
	do_rmdir 0 1 /dev/cgroup2
}

case5()
{
	exist_subsystem "cpuset"
	exist_subsystem "ns"
	exist_subsystem "memory"
	do_mount 1 1 "-odebug,cpuset,ns" /dev/cgroup cgroup1
	
	mount_str="`mount -l | grep /dev/cgroup2`"
	if [ "$mount_str" != "" ]; then
		do_umount 1 1 /dev/cgroup2
	fi
	
	if [ -e /dev/cgroup2 ]; then
		do_rmdir 0 1 /dev/cgroup2
	fi
	
	do_mkdir 0 1 /dev/cgroup2
	
	do_mount 0 1 "-odebug,cpuset,memory" /dev/cgroup2 cgroup2
	
	sleep 1
	
	do_umount 0 1 /dev/cgroup
	do_rmdir 0 1 /dev/cgroup
	do_umount 0 1 /dev/cgroup2
	do_rmdir 0 1 /dev/cgroup2
}

case6()
{
	exist_subsystem "debug"
	exist_subsystem "cpuset"
	exist_subsystem "ns"
	do_mount 1 1 "-odebug,cpuset,ns" /dev/cgroup cgroup1
	
	mount_str="`mount -l | grep /dev/cgroup2`"
	if [ "$mount_str" != "" ]; then
		do_umount 1 1 /dev/cgroup2
	fi
	
	if [ -e /dev/cgroup2 ]; then
		do_rmdir 0 1 /dev/cgroup2
	fi
	
	do_mkdir 0 1 /dev/cgroup2
	
	do_mount 0 1 "-oall" /dev/cgroup2 cgroup2
	
	sleep 1
	
	do_umount 0 1 /dev/cgroup
	do_rmdir 0 1 /dev/cgroup
	do_umount 0 1 /dev/cgroup2
	do_rmdir 0 1 /dev/cgroup2
}

case7()
{
	do_mkdir 0 1 /dev/cgroup/subgroup_2
	
	do_mv 0 1 /dev/cgroup/subgroup_1 /dev/cgroup/subgroup_3
}

case8()
{
	do_mkdir 0 1 /dev/cgroup/subgroup_2
	
	do_mv 0 0 /dev/cgroup/subgroup_1 /dev/cgroup/subgroup_2
}

case9()
{
	mount_str="`mount -l | grep /dev/cgroup2`"
	if [ "$mount_str" != "" ]; then
		do_umount 1 1 /dev/cgroup2
	fi
	
	if [ -e /dev/cgroup2 ]; then
		do_rmdir 0 1 /dev/cgroup2
	fi
	
	do_mkdir 0 1 /dev/cgroup2
	
	do_mkdir 0 1 /dev/cgroup2/subgroup_2
	
	do_mv 0 1 /dev/cgroup/subgroup_1 /dev/cgroup/subgroup_2

	sleep 1

	do_rmdir 0 1 /dev/cgroup2/subgroup_2
	do_rmdir 0 1 /dev/cgroup2
}

case10()
{
	do_mkdir 0 1 /dev/cgroup/subgroup_2
	
	do_mv 0 0 /dev/cgroup/subgroup_1 /dev/cgroup/tasks
}

case11()
{
	do_echo 0 1 $pid /dev/cgroup/subgroup_1/tasks
	
	sleep 1
	
	do_rmdir 0 0 /dev/cgroup/subgroup_1
	
	sleep 1
	
	do_echo 1 1 $pid /dev/cgroup/tasks
}

case12()
{
	do_mkdir 0 1 /dev/cgroup/subgroup_1/subgroup_1_1
	
	sleep 1
	
	do_rmdir 0 0 /dev/cgroup/subgroup_1
	
	do_rmdir 1 1 /dev/cgroup/subgroup_1/subgroup_1_1
}

case13()
{
	do_rmdir 0 1 /dev/cgroup/subgroup_1
}

##########################  main   #######################
if [ "$#" -ne "1" ] || [ $caseno -lt 1 ] || [ $caseno -gt 13 ]; then
	usage;
	exit_parameter;
fi

echo "-------------------------------------------------------------------------" >> $LOGFILE
echo "case no : $CASENO1" >> $LOGFILE
echo `date` >> $LOGFILE

exist_subsystem "debug"	
setup;

echo "INFO: now we begin to test no $CASENO1 ..." >> $LOGFILE

if [ $caseno -lt 3 ] || [ $caseno -gt 6 ]; then
	mount_cgroup;
	$TESTROOT/cgroup_fj_proc &
	pid=$!
	mkdir_subgroup;
fi

case$caseno

cleanup;
if [ $caseno -lt 3 ] || [ $caseno -gt 6 ]; then
	do_kill 1 1 9 $pid
fi
sleep 1	
exit 0;
