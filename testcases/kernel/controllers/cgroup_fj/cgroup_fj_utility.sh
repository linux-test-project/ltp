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

subsystem_str=""

exist_subsystem()
{
	checksubsystem=""
	case "$#" in
	"0" )
		checksubsystem=$subsystem_str
		;;
	"1" )
		checksubsystem=$1
		;;
	"*" )
		if [ "$#" -gt "1" ]; then
			exit -1
		fi
		;;
	esac

	if [ "$checksubsystem" = "" ]; then
		exit -1
	fi
	
	exist=`grep -w $checksubsystem /proc/cgroups | cut -f1`;
	if [ "$exist" = "" ]; then
		exit 9 
	fi
}

get_subsystem()
{
	case $subsystem in
	"1" )
		subsystem_str="debug";
		exist_subsystem;
		;;
	"2" )
		subsystem_str="cpuset";
		exist_subsystem;
		;;
	"3" )
		subsystem_str="ns";
		exist_subsystem;
		;;
	"4" )
		subsystem_str="cpu"
		exist_subsystem;
		;;
	"5" )
		subsystem_str="cpuacct";
		exist_subsystem;
		;;
	"6" )
		subsystem_str="memory";
		exist_subsystem;
		;;
	"7" )
		subsystem_str="all";
		;;
	"8" )
		subsystem_str=""
		;;
	"9" )
		subsystem_str="debug,debug";
		exist_subsystem "debug";
		;;
	"10" )
		subsystem_str="abc";
		;;
	"11" )
		subsystem_str="freezer";
		exist_subsystem;
		;;
	"12" )
		subsystem_str="devices";
		exist_subsystem;
		;;
	 *  )
		return -1
		;;
	esac
}

get_remount_use()
{
	case $remount_use in
	"1" )
		remount_use_str="";
		;;
	"2" )
		remount_use_str="remount";
		;;
	 *  )
		return -1
		;;
	esac
}

get_noprefix_use()
{
	case $noprefix_use in
	"1" )
		noprefix_use_str="";
		;;
	"2" )
		if [ $subsystem -ne 2 ]; then
			return -1
		fi
		noprefix_use_str="noprefix";
		;;
	 *  )
		return -1
		;;
	esac
}

get_release_agent_para()
{
	case $release_agent_para in
	"1" )
		release_agent_para_str="";
		;;
	"2" )
		release_agent_para_str=" ";
		;;
	"3" )
		release_agent_para_str="/root/cgroup_fj_release_agent";
		;;
	"4" )
		release_agent_para_str="cgroup_fj_release_agent";
		;;
	"5" )
		release_agent_para_str="/sbin/cgroup_fj_release_agent";
		;;
	"6" )
		release_agent_para_str="/root/cgroup_fj_release_agent";
		;;
	"7" )
		release_agent_para_str="/sbin/abc";
		;;
	"8" )
		release_agent_para_str="/sbin/cgroup_fj_release_agent";
		;;
	 *  )
		return -1
		;;
	esac
}

get_notify_on_release()
{
	case $notify_on_release in
	"1" )
		notify_on_release_str="0";
		;;
	"2" )
		notify_on_release_str="1";
		;;
	"3" )
		notify_on_release_str="2";
		;;
	"4" )
		notify_on_release_str="-1";
		;;
	"5" )
		notify_on_release_str="0ddd";
		;;
	"6" )
		notify_on_release_str="1ddd";
		;;
	"7" )
		notify_on_release_str="ddd1";
		;;
	 *  )
		return -1
		;;
	esac
}

get_release_agent()
{
	case $release_agent_echo in
	"1" )
		release_agent_str="";
		;;
	"2" )
		release_agent_str="/root/cgroup_fj_release_agent";
		;;
	"3" )
		release_agent_str="cgroup_fj_release_agent";
		;;
	"4" )
		release_agent_str="/sbin/cgroup_fj_release_agent";
		;;
	"5" )
		release_agent_str="/root/cgroup_fj_release_agent";
		;;
	"6" )
		release_agent_str="/sbin/abc";
		;;
	"7" )
		release_agent_str="/sbin/cgroup_fj_release_agent";
		;;
	 *  )
		return -1
		;;
	esac
}

# check the exit status, and exit or echo info.
# exit status	expectted value	echo info	exit or not
# 0		0		no		no
# 0		1		yes	yes
# not 0	0		yes	yes
# not 0	1		no		no
do_exit()
{
	if [ "$#" -ne "3" ]; then
		echo "ERROR: exit failed,your parameter is wrong..Exiting test" >> $LOGFILE
		exit -1
	fi
	
	exit_here=$1
	expectted=$2
	exit_status=$3
	
	if [ $exit_status -eq 0 ] ;then
		if [ $expectted -lt 1 ]; then
			echo "								against with expectted" >> $LOGFILE
			if [ $exit_here -ge 1 ]; then
				cleanup;
				exit -1
			fi
		fi
	else
		if [ $expectted -ge 1 ]; then
			echo "								against with expectted" >> $LOGFILE
			if [ $exit_here -ge 1 ]; then
				cleanup;
				exit -1
			fi
		else
			if [ $exit_here -ge 1 ]; then
				cleanup;
				exit 0
			fi
		fi
	fi
}

do_echo()
{
	if [ "$#" -ne "4" ]; then
		echo "ERROR: echo failed,your parameter is wrong..Exiting test" >> $LOGFILE
		exit -1
	fi
	
	exit_here=$1
	expectted=$2
	value=$3
	target=$4

	if [ $no_debug -ne 1 ]; then
		if [ $expectted -ge 1 ]; then
			echo "\"echo $value > $target\" (expectted: success)" >> $LOGFILE
		else
			echo "\"echo $value > $target\" (expectted: fail)" >> $LOGFILE
		fi
	fi

	`echo $value > $target` >> $LOGFILE 2>&1
	do_exit $exit_here $expectted $?;
}

do_mkdir()
{
	if [ "$#" -ne "3" ] && [ "$#" -ne "4" ]; then
		echo "ERROR: mkdir failed,your parameter is wrong..Exiting test" >> $LOGFILE
		exit -1
	fi
	
	exit_here=$1
	expectted=$2
	target=$3
	parents=0
	if [ "$#" -eq "4" ] && [ "$4" -ne 0 ]; then
		parents=1
	fi

	if [ $no_debug -ne 1 ]; then
		if [ $expectted -ge 1 ]; then
			echo "\"mkdir $target\" (expectted: success)" >> $LOGFILE
		else
			echo "\"mkdir $target\" (expectted: fail)" >> $LOGFILE
		fi
	fi
	
	if [ $parents -ne "1" ] && [ -e $target ]; then
		do_rmdir $exit_here 1 $target
	fi
	
	if [ $parents -ne "1" ]; then
		mkdir $target >> $LOGFILE 2>&1
	else
		mkdir -p $target >> $LOGFILE 2>&1
	fi
	do_exit $exit_here $expectted $?;
}

do_rmdir()
{
	if [ "$#" -lt "3" ]; then
		echo "ERROR: rmdir failed,your parameter is wrong..Exiting test" >> $LOGFILE
		exit -1
	fi
	
	exit_here=$1
	expectted=$2
	target=$3

	if ! [ -e $target ]; then
		echo "INFO: $target is not exist" >> $LOGFILE
		return
	fi
	
	if [ $no_debug -ne 1 ]; then
		if [ $expectted -ge 1 ]; then
			echo "\"rmdir $target\" (expectted: success)" >> $LOGFILE
		else
			echo "\"rmdir $target\" (expectted: fail)" >> $LOGFILE
		fi
	fi
	
	rmdir $3 $4 $5 >> $LOGFILE 2>&1
	do_exit $exit_here $expectted $?;
}

do_mount()
{
	if [ "$#" -ne "4" ] && [ "$#" -ne "5" ] ; then
		echo "ERROR: mount failed,your parameter is wrong..Exiting test" >> $LOGFILE
		exit -1
	fi
	
	exit_here=$1
	expectted=$2
	para_o=$3
	target=$4
	something="cgroup"
	if [ "$#" -eq "5" ]; then
		something=$5
	fi
	
	if [ $no_debug -ne 1 ]; then
		if [ $expectted -ge 1 ]; then
			echo "\"mount -t cgroup $para_o $something $target\" (expectted: success)" >> $LOGFILE
		else
			echo "\"mount -t cgroup $para_o $something $target\" (expectted: fail)" >> $LOGFILE
		fi
	fi
	
	mount -t cgroup $para_o $something $target >> $LOGFILE 2>&1
	do_exit $exit_here $expectted $?;
}

do_umount()
{
	if [ "$#" -ne "3" ]; then
		echo "ERROR: umount failed,your parameter is wrong..Exiting test" >> $LOGFILE
		exit -1
	fi
	
	exit_here=$1
	expectted=$2
	target=$3
	
	if [ $no_debug -ne 1 ]; then
		if [ $expectted -ge 1 ]; then
			echo "\"umount $target\" (expectted: success)" >> $LOGFILE
		else
			echo "\"umount $target\" (expectted: fail)" >> $LOGFILE
		fi
	fi
	
	umount $target >> $LOGFILE 2>&1
	do_exit $exit_here $expectted $?;
}

do_mv()
{
	if [ "$#" -ne "4" ]; then
		echo "ERROR: mv failed,your parameter is wrong..Exiting test" >> $LOGFILE
		exit -1
	fi
	
	exit_here=$1
	expectted=$2
	source=$3
	target=$4
	
	if [ $no_debug -ne 1 ]; then
		if [ $expectted -ge 1 ]; then
			echo "\"mv $source $target\" (expectted: success)" >> $LOGFILE
		else
			echo "\"mv $source $target\" (expectted: fail)" >> $LOGFILE
		fi
	fi
	
	mv $source $target >> $LOGFILE 2>&1
	do_exit $exit_here $expectted $?;
}

do_kill()
{
	if [ "$#" -ne "4" ]; then
		echo "ERROR: kill failed,your parameter is wrong..Exiting test" >> $LOGFILE
		exit -1
	fi
	
	exit_here=$1
	expectted=$2
	signo=$3
	pid=$4

	if ! [ -e /proc/$pid/ ];then
		return
	fi	

	if [ $no_debug -ne 1 ]; then
		if [ $expectted -ge 1 ]; then
			echo "\"kill -$signo $pid\" (expectted: success)" >> $LOGFILE
		else
			echo "\"kill -$signo $pid\" (expectted: fail)" >> $LOGFILE
		fi
	fi

	kill -s $signo $pid >> $LOGFILE 2>&1
	do_exit $exit_here $expectted $?;
}

setup()
{
	if [ -e /dev/cgroup ]; then
		cleanup;
	fi
	do_mkdir 1 1 /dev/cgroup
	
	if [ -e $TESTROOT/cgroup_fj_release_agent ]
	then
		cp -f $TESTROOT/cgroup_fj_release_agent /sbin
		chmod a+x /sbin/cgroup_fj_release_agent
		cp -f $TESTROOT/cgroup_fj_release_agent /root
		chmod a+x /root/cgroup_fj_release_agent
	else
		echo "ERROR: $TESTROOT/cgroup_fj_release_agent isn't exist..Exiting test" >> $LOGFILE
		exit -1;
	fi

	if [ $release_agent_para -eq 8 ] || [ $release_agent_echo -eq 7 ]; then
		chmod a-x /sbin/cgroup_fj_release_agent
	fi
	
	if [ -e $TESTROOT/cgroup_fj_proc ]
	then
		chmod a+x $TESTROOT/cgroup_fj_proc
	else
		echo "ERROR: $TESTROOT/cgroup_fj_proc isn't exist..Exiting test" >> $LOGFILE
		exit -1;
	fi
}

cleanup()
{
	if [ $no_debug -ne 1 ]; then
		echo "INFO: we now cleanup ..." >> $LOGFILE
	fi

	export LANG=en_US.UTF-8

	rm -f /sbin/cgroup_fj_release_agent 2>/dev/null
	rm -f /root/cgroup_fj_release_agent 2>/dev/null

	killall -9 cgroup_fj_proc 1>/dev/null 2>&1;

	if [ -e /dev/cgroup/subgroup_1 ]; then
		cat /dev/cgroup/subgroup_1/tasks > $TMPFILE
		nlines=`cat $TMPFILE | wc -l`
		for i in `seq 1 $nlines`
		do
			cur_pid=`sed -n "$i""p" $TMPFILE`
			if [ -e /proc/$cur_pid/ ];then
				do_echo 0 1 "$cur_pid" /dev/cgroup/tasks
			fi
		done
		do_rmdir 0 1 /dev/cgroup/subgroup_*
	fi
	
	if [ -e $TMPFILE ]; then
		rm -f $TMPFILE 2>/dev/null
	fi

	mount_str="`mount -l | grep /dev/cgroup`"
	if [ "$mount_str" != "" ]; then
		do_umount 0 1 /dev/cgroup
	fi
	
	if [ -e /dev/cgroup ]; then
		do_rmdir 0 1 /dev/cgroup
	fi
}

reclaim_foundling()
{
	if ! [ -e /dev/cgroup/subgroup_1 ]; then
		return
	fi
	foundlings=0
	cat `find /dev/cgroup/subgroup_* -name "tasks"` > $TMPFILE
	nlines=`cat "$TMPFILE" | wc -l`
	for k in `seq 1 $nlines`
	do
		cur_pid=`sed -n "$k""p" $TMPFILE`
		if [ -e /proc/$cur_pid/ ];then
			echo "ERROR: pid $cur_pid reclaimed"
			do_echo 0 1 "$cur_pid" "/dev/cgroup/tasks"
			: $((foundlings += 1))
		fi
	done

	if [ $foundlings -ne 0 ]; then
		echo "ERROR: some processes lost in subgroups's tasks, now they are reclaimed."
	fi 
}

mkdir_subgroup()
{
	if ! [ -e /dev/cgroup ]; then
		echo "ERROR: /dev/cgroup isn't exist..Exiting test" >> $LOGFILE
		exit -1;
	fi

	do_mkdir 1 1 /dev/cgroup/subgroup_1
}

mount_cgroup ()
{
	expectted=1
	PARAMETER_O="";
	if [ $subsystem -eq 10 ]; then
		expectted=0
	fi
	if [ "$subsystem_str" != "" ]; then
		PARAMETER_O="$subsystem_str"
	fi
	if [ "$noprefix_use_str" != "" ]; then
		if [ "$PARAMETER_O" != "" ]; then
			PARAMETER_O="$PARAMETER_O"",""$noprefix_use_str"
		else
			PARAMETER_O="$noprefix_use_str"
		fi
	fi
	if [ "$release_agent_para_str" != "" ]; then
		if [ "$PARAMETER_O" != "" ]; then
			PARAMETER_O="$PARAMETER_O"",release_agent=""$release_agent_para_str"
		else
			PARAMETER_O="release_agent=""$release_agent_para_str"
		fi
	fi
	if [ "$remount_use_str" != "" ]; then
		if [ "$PARAMETER_O" != "" ]; then
			do_mount 1 1 "-o$PARAMETER_O" /dev/cgroup
			PARAMETER_O="$PARAMETER_O"",""$remount_use_str"
		else
			do_mount 1 1 "" /dev/cgroup
			PARAMETER_O="$remount_use_str"
		fi
		sleep 1
	fi

	if [ "$PARAMETER_O" != "" ]; then
		PARAMETER_O="-o""$PARAMETER_O"
	fi
	
	do_mount 1 $expectted "$PARAMETER_O" /dev/cgroup
}

check_para()
{
	get_subsystem;
	ret1=$?
	get_remount_use;
	ret2=$?
	get_noprefix_use;
	ret3=$?
	get_release_agent_para;
	ret4=$?
	get_notify_on_release;
	ret5=$?
	get_release_agent;
	ret6=$?

	if [ $ret1 -ne 0 ] || [ $ret2 -ne 0 ] || [ $ret3 -ne 0 ] || [ $ret4 -ne 0 ] || [ $ret5 -ne 0 ] || [ $ret6 -ne 0 ] 
	then
		echo "ERROR: Wrong inputed parameter..Exiting test" >> $LOGFILE
		return -1
	fi

	return 0
}
