#!/bin/bash
# usage ./runcpuctl_test.sh

#################################################################################
#  Copyright (c) International Business Machines  Corp., 2007                   #
#                                                                               #
#  This program is free software;  you can redistribute it and/or modify        #
#  it under the terms of the GNU General Public License as published by         #
#  the Free Software Foundation; either version 2 of the License, or            #
#  (at your option) any later version.                                          #
#                                                                               #
#  This program is distributed in the hope that it will be useful,              #
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of              #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                    #
#  the GNU General Public License for more details.                             #
#                                                                               #
#  You should have received a copy of the GNU General Public License            #
#  along with this program;  if not, write to the Free Software                 #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA      #
#                                                                               #
#################################################################################
# Name Of File: run_cpuctl_test.sh                                              #
#                                                                               #
# Description: 	This file runs the setup for testing cpucontroller.             #
#               After setup it runs some of the tasks in different groups.      #
#               setup includes creating controller device, mounting it with     #
#               cgroup filesystem with option cpu and creating groups in it.    #
#                                                                               #
# Functions:    get_num_groups(): decides number of groups based on num of cpus	#
#               setup(): creaes /dev/cpuctl, mounts cgroup fs on it, creates 	#
#               groups in that, creates fifo to fire tasks at one time.         #
#                                                                               #
# Precaution:   Avoid system use by other applications/users to get fair and    #
#               appropriate results                                             #
#                                                                               #
# Author:       Sudhir Kumar   <sudhirkumarmalik@In.ibm.com>                    #
#                                                                               #
# History:                                                                      #
#                                                                               #
#  DATE         NAME           EMAIL                	     DESC               #
#                                                                               #
#  20/12/07  Sudhir Kumar <sudhirkumarmalik@in.ibm.com>   Created this test	#
#                                                                               #
#################################################################################


export TCID="cpuctl_test01";
export TST_TOTAL=1;
export TST_COUNT=1;

RC=0;			# return code from functions
NUM_CPUS=1;		# at least 1 cpu is there
NUM_GROUPS=2;		# min number of groups

PWD=`pwd`
cd $LTPROOT/testcases/bin/

get_num_groups()	# Number of tasks should be >= number of cpu's (to check scheduling fairness)
{
	NUM_CPUS=`cat /proc/cpuinfo | grep -w processor | wc -l`
	num_grps=$(echo "$NUM_CPUS * 1.5"|bc)	# temp variable num_grps
	int_part=`echo $num_grps | cut -d"." -f1`
	dec_part=`echo $num_grps | cut -d"." -f2`

	if [ $dec_part -gt 0 ]
	then
		NUM_GROUPS=$(echo "$int_part + 1"|bc)
	else
		NUM_GROUPS=$int_part;
	fi
}

# Write the cleanup function
cleanup ()
{
	echo "Cleanup called"
	rm -f cpuctl_task_* 2>/dev/null
	rmdir /dev/cpuctl/group_* 2> /dev/null
	umount /dev/cpuctl 2> /dev/null
	rmdir /dev/cpuctl 2> /dev/null
	rm -f myfifo 2>/dev/null

}
	# Create /dev/cpuctl &  mount the cgroup file system with cpu controller
	#clean any group created eralier (if any)

setup ()
{
	if [ -e /dev/cpuctl ]
	then
		echo "WARN:/dev/cpuctl already exist..overwriting"; # or a warning ?
		cleanup;
		mkdir /dev/cpuctl;
	else
		mkdir /dev/cpuctl
	fi
	mount -t cgroup -ocpu cgroup /dev/cpuctl 2> /dev/null
	if [ $? -ne 0 ]
	then
		echo "ERROR: Could not mount cgroup filesystem on /dev/cpuctl..Exiting test";
		cleanup;
		exit -1;
	fi

	# Group created earlier may again be visible if not cleaned properly...so clean them
	if [ -e /dev/cpuctl/group_1 ]
	then
		rmdir /dev/cpuctl/group*
		echo "WARN: Earlier groups found and removed...";
	fi

	#Create a fifo to make all tasks wait on it
	mkfifo myfifo 2> /dev/null;
	if [ $? -ne 0 ]
	then
		echo "ERROR: Can't create fifo...Check your permissions..Exiting test";
		cleanup;
		exit -1;
	fi

	# Create different groups
	i=1;
	while [ $i -le $NUM_GROUPS ]
	do
		group=group_$i;
		mkdir /dev/cpuctl/$group;# 2>/dev/null
		if [ $? -ne 0 ]
		then
			echo "ERROR: Can't create $group...Check your permissions..Exiting test";
			cleanup;
			exit -1;
		fi
		i=`expr $i + 1`
	done
}

##########################  main   #######################
	echo "TEST: CPU CONTROLLER TESTING";
	echo "RUNNING SETUP.....";
	get_num_groups;
	if [ $NUM_GROUPS -lt 2 ]
	then
		NUM_GROUPS=2;	# min num of groups for testing
	fi

	setup;

	# Trap the signal from any abnormaly terminated task
	# and kill all others and let cleanup be called
	trap 'echo "signal caught from task"; killall cpuctl_task_*;' SIGUSR1;

	echo "TEST STARTED: Please avoid using system while this test executes";
	#Check if  c source  file has been compiled and then run it in different groups
	if [ -f cpuctl_test01 ]
	then
		echo `date` >> $LTPROOT/output/cpuctl_results.txt;
		for i in $(seq 1 $NUM_GROUPS)
		do
			cp cpuctl_test01 cpuctl_task_$i 2>/dev/null;
			chmod +x cpuctl_task_$i;
			./cpuctl_task_$i $i /dev/cpuctl/group_$i $$ $NUM_CPUS >>$LTPROOT/output/cpuctl_results.txt 2>/dev/null &
			if [ $? -ne 0 ]
			then
				echo "Error: Could not run ./cpuctl_task_$i"
				cleanup;
				exit -1;
			else
				PID[$i]=$!;
			fi
			i=`expr $i + 1`
		done
	else
		echo "Source file not compiled..Plz check Makefile...Exiting test"
		cleanup;
		exit -1;
	fi
	sleep 3
	echo TASKS FIRED
	echo helloworld > myfifo;

	#wait for the tasks to finish for cleanup and status report to pan
	for i in $(seq 1 $NUM_GROUPS)
	do
		wait ${PID[$i]};
		RC=$?;	# Return status of the task being waited
		# In abnormal termination of anyone trap will kill all others
		# and they will return non zero exit status. So Test broke!!
		if [ $RC -ne 0 ]
		then
			echo "Task $i exited abnormalywith return value: $RC";
			tst_resm TINFO "Test could not execute for the expected duration";
			cleanup;
			exit -1;
		fi
	done
	echo "Cpu controller test executed successfully.Results written to file";
	echo "Please review the results in $LTPROOT/output/cpuctl_results.txt"
	cleanup;
	cd $PWD
	exit 0;		#to let PAN reprt success of test
