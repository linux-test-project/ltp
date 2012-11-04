#!/bin/bash
# usage ./runcpuctl_stress_test.sh test_num

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
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA      #
#                                                                               #
#################################################################################
# Name Of File: run_cpuctl_stress_test.sh                                       #
#                                                                               #
# Description:  This file runs the setup for testing cpucontroller.             #
#               After setup it runs some of the tasks in different groups.      #
#               setup includes creating controller device, mounting it with     #
#               cgroup filesystem with option cpu and creating groups in it.    #
#               This same script can run 4 testcases depending on test number   #
#               depending on the test number passed by the calling script.      #
#                                                                               #
# Test 06:      N X M (N groups with M tasks each)                              #
# Test 07:      N*M X 1 (N*M groups with 1 task each)                           #
# Test 08:      1 X N*M (1 group with N*M tasks)                                #
# Test 09:      Heavy stress test with nice value change                        #
# Test 10:      Heavy stress test (effect of heavy group on light group)        #
#                                                                               #
# Precaution:   Avoid system use by other applications/users to get fair and    #
#               appropriate results                                             #
#                                                                               #
# Author:       Sudhir Kumar   <skumar@linux.vnet.ibm.com>                      #
#                                                                               #
# History:                                                                      #
#                                                                               #
#  DATE         NAME           EMAIL                         DESC               #
#                                                                               #
#  20/12/07  Sudhir Kumar <skumar@linux.vnet.ibm.com>   Created this test       #
#                                                                               #
#################################################################################


export TCID="cpuctl_test06";
export TST_TOTAL=4;
export TST_COUNT=1;	# how to tell here ??

RC=0;			# return code from functions
NUM_CPUS=1;		# at least 1 cpu is there
NUM_GROUPS=2;		# min number of groups
TEST_NUM=$1;            # To run the desired test (1 or 2)
TASK_NUM=0;		# The serial number of a task
TOTAL_TASKS=0;		# Total num of tasks in any test
TASKS_IN_GROUP=0;	# Total num of tasks in a group
NICEVALUE=0;
SCRIPT_PID=$$;
FILE="stress-678";		# suffix for results file
TEST_NAME="CPUCTL NUM_GROUPS vs NUM_TASKS TEST:";

NUM_CPUS=`tst_ncpus`
N=$NUM_CPUS;		# Default total num of groups (classes)
M=10;			# Default total num of tasks in a group

PWD=`pwd`
cd $LTPROOT/testcases/bin/

. parameters.sh

usage ()
{
  	echo "Could not start cpu controller stress test";
	echo "Check entry in file $LTPROOT/testcases/kernel/controllers/test_controllers.sh";
	echo "usage: run_cpuctl_stress_test.sh test_num";
	echo "Skipping the test...";
	exit -1;
}
##########################  main   #######################
		# For testcase 1, 2 & 3 N--> $NUM_CPUS
		# 1,2 & 3 are not heavy stress test

	case ${TEST_NUM} in

	"6" )	# N X M (N groups with M tasks each)
		if [ $N -eq 1 ]
		then
			N=2;	# Min 2 groups for group scheduling
		fi;
		NUM_GROUPS=$N;
		TASKS_IN_GROUP=$M;
		echo `date` >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		;;
	"7" )   # N*M X 1 (N*M groups with 1 task each)
		if [ $N -eq 1 ]
		then
			N=2;	# To keep total tasks same as in case 1
		fi;
		NUM_GROUPS=`expr $N \* $M`;
		TASKS_IN_GROUP=1;
		;;
	"8" )	# 1 X N*M (1 group with N*M tasks)
		if [ $N -eq 1 ]
		then
			N=2;	# To keep total tasks same as in case 1
		fi;
		NUM_GROUPS=1;
		TASKS_IN_GROUP=`expr $N \* $M`;
		;;
	"9" )	# Heavy stress test
		NUM_GROUPS=`expr $N \* $M`;
		TASKS_IN_GROUP=`expr 1 \* $M`;
		FILE="stress-9";
		TEST_NAME="HEAVY STRESS TEST(RENICED):";
		echo `date` >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		;;
	"10" )	# Heavy stress test
		NUM_GROUPS=2;
		M=`expr $N \* 100`;
		FILE="stress-10";
		TEST_NAME="LIGHT GRP vs HEAVY GRP TEST:";
		echo `date` >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		;;
	  * )
		usage;
		;;
	esac

	echo "TEST $TEST_NUM: CPU CONTROLLER STRESS TESTING";
	echo "RUNNING SETUP.....";
	do_setup;

	# Trap the signal from any abnormaly terminated task
	# and kill all others and let cleanup be called
	trap 'echo "signal caught from task"; killall cpuctl_task_*;' SIGUSR1;

	echo "TEST STARTED: Please avoid using system while this test executes";
	#Check if  c source  file has been compiled and then run it in different groups

	case $TEST_NUM in

	"6" | "7" | "8" )

		if [ -f cpuctl_test03 ]
		then
		echo TEST NAME:- $TEST_NAME: $TEST_NUM >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo Test $TEST_NUM: NUM_GROUPS=$NUM_GROUPS +1 \(DEF\)>> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo Test $TEST_NUM: TASKS PER GROUP=$TASKS_IN_GROUP >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo "==========================================" >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		for i in $(seq 1 $NUM_GROUPS)
		do
			MYGROUP=/dev/cpuctl/group_$i
			for j in $(seq 1 $TASKS_IN_GROUP)
			do
			TASK_NUM=`expr $TASK_NUM + 1`;
			cp cpuctl_test03 cpuctl_task_$TASK_NUM ;
			chmod +x cpuctl_task_$TASK_NUM;

			GROUP_NUM=$i MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID NUM_CPUS=$NUM_CPUS \
			TEST_NUM=$TEST_NUM TASK_NUM=$TASK_NUM ./cpuctl_task_$TASK_NUM \
			>>$LTPROOT/output/cpuctl_results_$FILE.txt &
			if [ $? -ne 0 ]
			then
				echo "Error: Could not run ./cpuctl_task_$TASK_NUM"
				cleanup;
				exit -1;
			else
				PID[$TASK_NUM]=$!;
			fi;
			j=`expr $j + 1`
			done;		# end j loop
			i=`expr $i + 1`
		done;			# end i loop
		else
			echo "Source file not compiled..Plz check Makefile...Exiting test"
			cleanup;
			exit -1;
		fi;
		TOTAL_TASKS=$TASK_NUM;
		# Run the default task in a default group
		set_def_group;
		if [ ! -f cpuctl_def_task03 ]; then
			echo "Source file for default task not compiled";
			echo "Plz check Makefile...Exiting test";
			cleanup;
			exit -1;
		fi
		MYGROUP=/dev/cpuctl/group_def ;
		GROUP_NUM=0 MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID \
		NUM_CPUS=$NUM_CPUS TEST_NUM=$TEST_NUM TASK_NUM=0 \
		./cpuctl_def_task03 >>$LTPROOT/output/cpuctl_results_$FILE.txt &
		if [ $? -ne 0 ]
		then
			echo "Error: Could not run ./cpuctl_def_task03"
			cleanup;
			exit -1;
		else
			echo "Succesfully launched def task $! too";
		fi
		;;
	"9" )

		if [ -f cpuctl_test04 ]
		then
		echo TEST NAME:- $TEST_NAME: $TEST_NUM >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo NUM_GROUPS=$NUM_GROUPS +1 \(DEF\)>> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo TASKS PER GROUP=$TASKS_IN_GROUP >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo "===============================" >> $LTPROOT/output/cpuctl_results_$FILE.txt;

		# Create 4 priority windows
		RANGE1=`expr $NUM_GROUPS / 4`;
		RANGE2=`expr $RANGE1 + $RANGE1`;
		RANGE3=`expr $RANGE2 + $RANGE1`;
		for i in $(seq 1 $NUM_GROUPS)
		do
			MYGROUP=/dev/cpuctl/group_$i
			for j in $(seq 1 $TASKS_IN_GROUP)
			do
			TASK_NUM=`expr $TASK_NUM + 1`;
			cp cpuctl_test04 cpuctl_task_$TASK_NUM ;
			chmod +x cpuctl_task_$TASK_NUM;

			# Per group nice value change must not affect group/task fairness
			if [ $i -le $RANGE1 ]
			then
				NICEVALUE=-16;
			elif [ $i -gt $RANGE1 ] && [ $i -le $RANGE2 ]
			then
				NICEVALUE=-17;
			elif [ $i -gt $RANGE2 ] && [ $i -le $RANGE3 ]
			then
				NICEVALUE=-18;
			else
				NICEVALUE=-19;
			fi

			GROUP_NUM=$i MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID NUM_CPUS=$NUM_CPUS \
			TEST_NUM=$TEST_NUM TASK_NUM=$TASK_NUM nice -n $NICEVALUE ./cpuctl_task_$TASK_NUM \
			>>$LTPROOT/output/cpuctl_results_$FILE.txt &
			if [ $? -ne 0 ]
			then
				echo "Error: Could not run ./cpuctl_task_$TASK_NUM"
				cleanup;
				exit -1;
			else
				PID[$TASK_NUM]=$!;
			fi;
			j=`expr $j + 1`
			done;		# end j loop
			i=`expr $i + 1`
		done;			# end i loop
		else
			echo "Source file not compiled..Plz check Makefile...Exiting test"
			cleanup;
			exit -1;
		fi;
		TOTAL_TASKS=$TASK_NUM;

		# Run the default task in a default group
		set_def_group;
		if [ ! -f cpuctl_def_task04 ]; then
			echo "Source file for default task not compiled";
			echo "Plz check Makefile...Exiting test";
			cleanup;
			exit -1;
		fi
		MYGROUP=/dev/cpuctl/group_def ;
		GROUP_NUM=0 MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID \
		NUM_CPUS=$NUM_CPUS TEST_NUM=$TEST_NUM TASK_NUM=0 \
		./cpuctl_def_task04 >>$LTPROOT/output/cpuctl_results_$FILE.txt &
		if [ $? -ne 0 ]
		then
			echo "Error: Could not run ./cpuctl_def_task04"
			cleanup;
			exit -1;
		else
			echo "Succesfully launched def task $! too";
		fi
		;;
	"10" )

		if [ -f cpuctl_test04 ]
		then
		echo TEST NAME:- $TEST_NAME: $TEST_NUM >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo NUM_GROUPS=$NUM_GROUPS +1 \(DEF\)>> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo TASKS PER GROUP=VARIABLE >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo "===============================" >> $LTPROOT/output/cpuctl_results_$FILE.txt;

		for i in $(seq 1 $NUM_GROUPS)
		do
			MYGROUP=/dev/cpuctl/group_$i;
			if [ $i -eq 1 ]
			then
				TASKS_IN_GROUP=$N;
			else
				TASKS_IN_GROUP=$M;
			fi;
			for j in $(seq 1 $TASKS_IN_GROUP)
			do
			TASK_NUM=`expr $TASK_NUM + 1`;
			cp cpuctl_test04 cpuctl_task_$TASK_NUM ;
			chmod +x cpuctl_task_$TASK_NUM;

			GROUP_NUM=$i MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID NUM_CPUS=$NUM_CPUS \
			TEST_NUM=$TEST_NUM TASK_NUM=$TASK_NUM ./cpuctl_task_$TASK_NUM \
			>>$LTPROOT/output/cpuctl_results_$FILE.txt &
			if [ $? -ne 0 ]
			then
				echo "Error: Could not run ./cpuctl_task_$TASK_NUM"
				cleanup;
				exit -1;
			else
				PID[$TASK_NUM]=$!;
			fi;
			j=`expr $j + 1`
			done;		# end j loop
			i=`expr $i + 1`
		done;			# end i loop
		else
			echo "Source file not compiled..Plz check Makefile...Exiting test"
			cleanup;
			exit -1;
		fi;
		TOTAL_TASKS=$TASK_NUM;

		# Run the default task in a default group
		set_def_group;
		if [ ! -f cpuctl_def_task04 ]; then
			echo "Source file for default task not compiled";
			echo "Plz check Makefile...Exiting test";
			cleanup;
			exit -1;
		fi
		MYGROUP=/dev/cpuctl/group_def ;
		GROUP_NUM=0 MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID \
		NUM_CPUS=$NUM_CPUS TEST_NUM=$TEST_NUM TASK_NUM=0 \
		./cpuctl_def_task04 >>$LTPROOT/output/cpuctl_results_$FILE.txt &
		if [ $? -ne 0 ]
		then
			echo "Error: Could not run ./cpuctl_def_task04"
			cleanup;
			exit -1;
		else
			echo "Succesfully launched def task $! too";
		fi
		;;
	  * )
		usage;
		;;
	esac

	sleep 8
	echo TASKS FIRED
	echo helloworld > myfifo;

	#wait for the tasks to finish for cleanup and status report to pan
	for i in $(seq 1 $TOTAL_TASKS)
	do
		wait ${PID[$i]};
		RC=$?;	# Return status of the task being waited
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
	echo "Cpu controller test executed successfully.Results written to file";
	echo "Please review the results in $LTPROOT/output/cpuctl_results_$FILE.txt"
	cleanup;
	cd $PWD
	exit 0;		#to let PAN reprt success of test
