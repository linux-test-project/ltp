#!/bin/bash
# usage ./runmemctl_test.sh test_num

#################################################################################
#  Copyright (c) International Business Machines  Corp., 2008                   #
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
# Name Of File: run_memctl_test.sh                                              #
#                                                                               #
# Description: This file runs the setup for testing different memory resource   #
#              controller features. After setup it runs diff test cases in diff #
#              setup.                                                           #
#                                                                               #
# Test 01:     Tests group memory usage on task migration                       #
# Test 03:     Tests failcnt increase on memory usage greater than group limit  #
#                                                                               #
# Precaution:   Avoid system use by other applications/users to get fair and    #
#               appropriate results (avoid unnecessary killing of applicatio)   #
#                                                                               #
# Author:       Sudhir Kumar   <skumar@linux.vnet.ibm.com>                      #
#                                                                               #
# History:                                                                      #
#                                                                               #
#  DATE         NAME           EMAIL                         DESC               #
#                                                                               #
#  12/03/08  Sudhir Kumar <skumar@linux.vnet.ibm.com>   Created this test       #
#  11/05/08  Sudhir Kumar <skumar@linux.vnet.ibm.com>   Added third test        #
#                                                                               #
#################################################################################

export TCID="memctl_test01-03";
export TST_TOTAL=3;
export TST_COUNT=1;

TEST_NUM=$1;
SCRIPT_PID=$$;
RC=0;
PWD=`pwd`;

check_mem_allocated()
{
#	MEM_TOTAL=$1;
	while [ 1 -gt 0 ]
	do
		sleep 1;
		USAGE_FROM_USAGE_IN_BYTES=`cat /dev/memctl/group_1/memory.usage_in_bytes`;
		if [ $USAGE_FROM_USAGE_IN_BYTES -gt $MEM_TOTAL ]
		then
			if [ $USAGE_FROM_USAGE_IN_BYTES -eq $SECOND_READ ]
			then
			# seems memory allocation is over now
				break;
			fi;
			sleep 5;
			SECOND_READ=`cat /dev/memctl/group_1/memory.usage_in_bytes`;
		fi
	done
}

cd $LTPROOT/testcases/bin/
. myfunctions.sh
#################################################################################
#   ****************************** WARNING *********************************    #
# User can change the parameters in different cases below but before doing      #
# any change user is supposed to know what he is doing. At any point when       #
# memory usage of a group becomes more than the group limit OOM Killer will     #
# be invoked and some of the tasks will be killed. Need to add code to handle   #
# the OOM Killer issues.                                                        #
#################################################################################

# First of all check if the system has sufficient memory
MEM_AVAIL=`cat /proc/meminfo | grep MemTotal | tr -s [:space:] | cut -d" " -f2`;
if [ $MEM_AVAIL -lt 262144 ]	# 256MB(as test requires some ~200MB)
then
	echo System does not have sufficient free memory.;
	echo Skipping execution of memory controller tests.;
	exit;
fi

case ${TEST_NUM} in

"1" | "2" )
	NUM_GROUPS=2;
	MEMLIMIT_GROUP_1=100M;
	MEMLIMIT_GROUP_2=132M;
	CHUNK_SIZE=6291456;				# malloc n chunks of size m(6M)
	NUM_CHUNKS=10;					# (say)60 MB memory(6*10)
	TOTAL_TASKS=1;					# num of tasks in a group(1)
	NUM_MIG_TASKS=$TOTAL_TASKS			# num of tasks to migrate
	MEM_TASK=`expr $CHUNK_SIZE \* $NUM_CHUNKS`;	# memory allocated by a task
	MEM_TOTAL=`expr $MEM_TASK \* $TOTAL_TASKS`;	# total memory allocated in a group
	TEST_NAME=" TASK MIGRATION TEST:";
	;;
"3" )
	NUM_GROUPS=1;
	MEMLIMIT_GROUP_1=50M;				# MEM_TASK > MEMLIMIT_GROUP_1
	CHUNK_SIZE=6291456;				# malloc n chunks of size m(6M)
	NUM_CHUNKS=10;					# (say)60 MB memory(6*10)
	TOTAL_TASKS=1;					# num of tasks in a group(1)
	MEM_TASK=`expr $CHUNK_SIZE \* $NUM_CHUNKS`;	# memory allocated by a task
	TEST_NAME=" FAILCNT CHECK TEST:";
	;;
"4" )
	NUM_GROUPS=1;
	MEMLIMIT_GROUP_1=100M;				# Memory limit of group1
	CHUNK_SIZE=6291456;				# malloc n chunks of size m(6M)
	NUM_CHUNKS=10;					# (say)60 MB memory(6*10)
	TOTAL_TASKS=1;					# num of tasks in a group(1)
	MEM_TASK=`expr $CHUNK_SIZE \* $NUM_CHUNKS`;	# memory allocated by a task
	MEM_TOTAL=`expr $MEM_TASK \* $TOTAL_TASKS`;	# total memory allocated in a group
	TEST_NAME=" STATS CHECK TEST:";
	SECOND_READ=0;
	;;
*  )	usage;
	exit -1
		;;
	esac

echo "TEST $TEST_NUM: MEMORY CONTROLLER TESTING";
echo "RUNNING SETUP.....";
setup;

# Trap the signal from any abnormaly terminated task
# and kill all others and let cleanup be called
trap 'echo "signal caught from task"; killall memctl_task_*;\
cleanup; exit -1;' SIGUSR1;#??? may need changes here

echo "TEST STARTED: Please avoid using system while this test executes";
#Check if  C source  file has been compiled and then run it in different groups

case $TEST_NUM in
"1" | "2" )
	setmemlimits;
	if [ -f memctl_test01 ]
	then
		for i in $(seq 1 $TOTAL_TASKS)
		do
			MYGROUP=/dev/memctl/group_1;
			cp memctl_test01 memctl_task_$i # 2>/dev/null;
			chmod +x memctl_task_$i;
			TEST_NUM=$TEST_NUM MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID CHUNK_SIZE=$CHUNK_SIZE \
				NUM_CHUNKS=$NUM_CHUNKS ./memctl_task_$i &
			if [ $? -ne 0 ]
			then
				echo "Error: Could not run ./memctl_task_$i"
				cleanup;
				exit -1;
			else
				PID[$i]=$!;
			fi
		done;	# tasks are now running in group1

		# Wait untill tasks allocate memory from group1
		while [ 1 -gt 0 ]
		do
			sleep 1;
			GRP1_MEMUSAGE=`cat /dev/memctl/group_1/memory.usage_in_bytes`;
			if [ $GRP1_MEMUSAGE -gt $MEM_TOTAL ]
			then
				break;
			fi
		done
		GRP2_MEMUSAGE_OLD=`cat /dev/memctl/group_2/memory.usage_in_bytes`;
		echo Before task migration to group2
		echo group2 memory usage: $GRP2_MEMUSAGE_OLD Bytes

		# Now migrate the tasks to another group
		for i in $(seq 1 $NUM_MIG_TASKS)
		do
			echo ${PID[$i]} >>/dev/memctl/group_2/tasks;
			if [ $? -ne 0 ]
			then
				echo "TFAIL Task migration failed from group_1 to group_2";
			fi
		done

		# double check
		GRP2_TASKS=`cat /dev/memctl/group_2/tasks|wc -l`;
		if [ $GRP2_TASKS -ne $NUM_MIG_TASKS ]
		then
			echo "TFAIL Task Migration failed for some of the tasks";
		fi;

		# Wait for some time to check if memory usage of group_2 increases
		# This is not the right approach however working. ??? thoughts???
		sleep 10;

		# Decision formula: decides PASS or FAIL
		case $TEST_NUM in
		"1" )
			GRP2_MEMUSAGE_NEW=`cat /dev/memctl/group_2/memory.usage_in_bytes`;
			echo After task migration to group2
			echo group2 memory usage: $GRP2_MEMUSAGE_NEW Bytes

			if [ $GRP2_MEMUSAGE_NEW -gt $GRP2_MEMUSAGE_OLD ]
			then
				echo "TFAIL   Memory resource Controller:  Task Migration test $TEST_NUM FAILED";
			else
				echo "TPASS   Memory Resource Controller: Task Migration test $TEST_NUM PASSED";
			fi

			# Now we can signal the task to finish and do the cleanup
			for i in $(seq 1 $TOTAL_TASKS)
			do
				kill -SIGUSR1 ${PID[$i]};
			done
			;;
		"2" )
			GRP2_MEMUSAGE_OLD=`cat /dev/memctl/group_2/memory.usage_in_bytes`;

			# signal the migrated tasks to allocate memory
			for i in $(seq 1 $TOTAL_TASKS)
			do
				kill -SIGUSR2 ${PID[$i]};
			done
			sleep 10; # Is it fine? Need input/alternates
			GRP2_MEMUSAGE_NEW=`cat /dev/memctl/group_2/memory.usage_in_bytes`;
			echo After task migration to group2 and doing malloc
			echo group2 memory usage: $GRP2_MEMUSAGE_NEW Bytes
			if [ $GRP2_MEMUSAGE_NEW -le $GRP2_MEMUSAGE_OLD ]
			then
				echo "TFAIL   Memory resource Controller:  Task Migration test $TEST_NUM FAILED";
			else
				# Now we can signal the task to finish and do the cleanup
				for i in $(seq 1 $TOTAL_TASKS)
				do
					kill -SIGUSR1 ${PID[$i]};
				done
				echo "TPASS   Memory Resource Controller: Task Migration test $TEST_NUM PASSED";
			fi

			;;
		esac

		else
		echo "Source file not compiled..Please check Makefile...Exiting test"
		cleanup;
		exit -1;
	fi;
	;;

"3" )
	setmemlimits;
	if [ -f memctl_test01 ]
	then
		MYGROUP=/dev/memctl/group_1;
		cp memctl_test01 memctl_task_1 # 2>/dev/null;
		chmod +x memctl_task_1;
		TEST_NUM=$TEST_NUM MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID CHUNK_SIZE=$CHUNK_SIZE \
			NUM_CHUNKS=$NUM_CHUNKS ./memctl_task_$i &
		if [ $? -ne 0 ]
		then
			echo "Error: Could not run ./memctl_task_1"
			cleanup;
			exit -1;
		else
			PID[1]=$!;
		fi

		# Wait untill tasks allocate memory from group1
		while [ 1 -gt 0 ]
		do
			sleep 1;
			GRP1_MEMUSAGE=`cat /dev/memctl/group_1/memory.usage_in_bytes`;
			MEMLIMIT_GROUP_1=`echo $MEMLIMIT_GROUP_1 | cut -d"M" -f1`;
			if [ $GRP1_MEMUSAGE -gt $MEMLIMIT_GROUP_1 ]
			then
				break;
			fi
		done
		# now memory.usage_in_bytes > memory.limit_in_bytes
		# hence memory.failcnt should be greater than 0
		FAILCNT=`cat /dev/memctl/group_1/memory.failcnt`;
		if [ $FAILCNT -gt 0 ]
		then
			# Now we can signal the task to finish and do the cleanup
			echo failcnt = $FAILCNT;
			kill -SIGUSR1 ${PID[1]};
			echo "TPASS   Memory Resource Controller: failcnt check test PASSED";
		else
			echo "TFAIL   Memory Resource Controller: failcnt check test FAILED";
		fi;
	else
		echo "Source file not compiled..Please check Makefile...Exiting test"
		cleanup;
		exit -1;
	fi;
		;;
"4" )
	setmemlimits;
	if [ -f memctl_test01 ]
	then
		MYGROUP=/dev/memctl/group_1;
		cp memctl_test01 memctl_task_1 # 2>/dev/null;
		chmod +x memctl_task_1;
		TEST_NUM=$TEST_NUM MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID \
			CHUNK_SIZE=$CHUNK_SIZE 	NUM_CHUNKS=$NUM_CHUNKS \
							./memctl_task_$i &
		if [ $? -ne 0 ]
		then
			echo "Error: Could not run ./memctl_task_1"
			cleanup;
			exit -1;
		else
			PID[1]=$!;
		fi

		# Wait untill tasks allocate memory from group1
		check_mem_allocated;# $MEM_TOTAL;
		# now we can check the memory usage from both files
		USAGE_FROM_STAT=`cat /dev/memctl/group_1/memory.stat \
					| grep -w "rss" | cut -d" " -f2`;
		if [ $USAGE_FROM_USAGE_IN_BYTES -eq $USAGE_FROM_STAT ]
		then
			echo "memory usage from memory.usage_in_bytes= $USAGE_FROM_USAGE_IN_BYTES";

			echo "memory usage from memory.stat= $USAGE_FROM_STAT";
			echo "TINFO   Memory Resource Controller: stat check test passes first run";

			echo Test continues to run the second step.
			FIRST_STEP_PASS=1;
		else
			echo "TINFO   Memory Resource Controller: stat check test fails in first run";
			FIRST_STEP_PASS=0;
		fi;
		kill -SIGUSR2 ${PID[1]};

		# just second run of the test with different memory allocations
		# Wait untill tasks allocate memory from group1
		while [ 1 -gt 0 ]
		do
			sleep 1;
			USAGE_FROM_USAGE_IN_BYTESGRP1_MEMUSAGE=`cat /dev/memctl/group_1/memory.usage_in_bytes`;

			if [ $USAGE_FROM_USAGE_IN_BYTESGRP1_MEMUSAGE -gt $MEM_TOTAL ]
			then
				if [ $USAGE_FROM_USAGE_IN_BYTESGRP1_MEMUSAGE -eq $SECOND_READ ]
				then
				# seems memory allocation is over now
					break;
				fi;
				sleep 5;
				SECOND_READ=`cat /dev/memctl/group_1/memory.usage_in_bytes`;
			fi
		done

		# now we can check the memory usage from both files
		USAGE_FROM_STAT=`cat /dev/memctl/group_1/memory.stat \
					| grep -w "rss" | cut -d" " -f2`;
		if [ $USAGE_FROM_USAGE_IN_BYTESGRP1_MEMUSAGE -eq $USAGE_FROM_STAT ] \
							&& [ $FIRST_STEP_PASS -eq 1 ]
		then
			# Now we can signal the task to finish and do cleanup
			kill -SIGUSR1 ${PID[1]};
			echo "memory usage from memory.usage_in_bytes= $USAGE_FROM_USAGE_IN_BYTESGRP1_MEMUSAGE";

			echo "memory usage from memory.stat=$USAGE_FROM_STAT";
			echo "TPASS   Memory Resource Controller: stat check test PASSED";
		else
			echo "TFAIL   Memory Resource Controller: stat check test FAILED";
		fi;
	else
		echo "Source file not compiled..Please check Makefile...Exiting test"
		cleanup;
		exit -1;
	fi;
		;;
	"*" )
		usage;
		exit -1;
		;;
	esac

	for i in $(seq 1 $TOTAL_TASKS)
	do
		wait ${PID[$i]};
		RC=$?;  # Return status of the task being waited
		# In abnormal termination of anyone trap will kill all others
		# and they will return non zero exit status. So Test broke!!
		if [ $RC -ne 0 ]
		then
			echo "Task $i exited abnormaly with return value: $RC";
			tst_resm TINFO "Test could not execute for the expected duration";
			cleanup;
			exit -1;
		fi
	done

	echo "Memory Resource Controller test executed successfully.";
	cleanup;
	cd $PWD
	exit 0;         #to let PAN reprt success of test
