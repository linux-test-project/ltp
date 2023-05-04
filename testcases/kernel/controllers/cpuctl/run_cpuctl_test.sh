#!/bin/bash
# usage ./runcpuctl_test.sh test_num

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
# Name Of File: run_cpuctl_test.sh                                              #
#                                                                               #
# Description: This file runs the setup for testing diff cpucontroller feature. #
#              After setup it runs diff test cases in diff setup.               #
#                                                                               #
# Test 01:     Tests fairness with respect to absolute share values             #
# Test 02:     Tests if fairness persists among different runs                  #
# Test 03:     Granularity test with respect to shares values                   #
# Test 04:     Nice value effect on group scheduling                            #
# Test 05:     Task migration test                                              #
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


export TCID="cpuctl_test01";
export TST_TOTAL=1;
export TST_COUNT=1;

RC=0;			# return code from functions
NUM_CPUS=1;		# at least 1 cpu is there
NUM_GROUPS=2;		# min number of groups
TEST_NUM=$1;            # To run the desired test (1 or 2)
TASK_NUM=0;		# The serial number of a task
TOTAL_TASKS=0;		# Total num of tasks in any test
TASKS_IN_GROUP=0	# Total num of tasks in a group
NICEVALUE=-20;		# Nice value to renice a task with
SCRIPT_PID=$$;
PWD=`pwd`
cd $LTPROOT/testcases/bin/
NUM_CPUS=`tst_ncpus`

. parameters.sh

##########################  main   #######################
	case ${TEST_NUM} in
	"1" )	get_num_groups;	# contains test case 1 and 2
		TEST_NAME="FAIRNESS TEST:"
		FILE="12";
		;;
	"3" )   NUM_GROUPS=`expr 2 \* $NUM_CPUS`;
		TEST_NAME="GRANULARITY TEST:";
		FILE=$TEST_NUM;
		;;
	"4" )   NUM_GROUPS=$NUM_CPUS;
		TEST_NAME="NICE VALUE TEST:";
		FILE=$TEST_NUM;
		;;
	"5" )   NUM_GROUPS=$NUM_CPUS;
		TEST_NAME=" TASK MIGRATION TEST:";
		FILE=$TEST_NUM;
		;;
	 *  )  	echo "Could not start cpu controller test";
		echo "usage: run_cpuctl_test.sh test_num";
		echo "Skipping the test...";
		exit -1;;
	esac
	echo "TEST $TEST_NUM: CPU CONTROLLER TESTING";
	echo "RUNNING SETUP.....";
	do_setup;

	# Trap the signal from any abnormally terminated task
	# and kill all others and let cleanup be called
	trap 'echo "signal caught from task"; killall cpuctl_task_*;' SIGUSR1;

	echo "TEST STARTED: Please avoid using system while this test executes";
	#Check if  c source  file has been compiled and then run it in different groups

	case $TEST_NUM in
	"1" | "3" )
		if [ -f cpuctl_test01 ]
		then
		echo `date` >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo `uname -a` >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo TEST:- $TEST_NAME $TEST_NUM:  >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo NUM_GROUPS=$NUM_GROUPS + 1\(DEF\) >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		for i in $(seq 1 $NUM_GROUPS)
		do
			cp cpuctl_test01 cpuctl_task_$i ;
			chmod +x cpuctl_task_$i;
			./cpuctl_task_$i $i /dev/cpuctl/group_$i $$ $NUM_CPUS $TEST_NUM \
			 >>$LTPROOT/output/cpuctl_results_$FILE.txt &
			if [ $? -ne 0 ]
			then
				echo "Error: Could not run ./cpuctl_task_$i"
				cleanup;
				exit -1;
			else
				PID[$i]=$!;
			fi
		done
		else
			echo "Source file not compiled..Plz check Makefile...Exiting test"
			cleanup;
			exit -1;
		fi;
		i=`expr $i + 1`

		TOTAL_TASKS=$NUM_GROUPS;
		# Run the default task in a default group
		set_def_group;
		if [ ! -f cpuctl_def_task01 ]; then
			echo "Source file for default task not compiled";
			echo "Plz check Makefile...Exiting test";
			cleanup;
			exit -1;
		fi
		./cpuctl_def_task01 $i /dev/cpuctl/group_def $$ $NUM_CPUS \
		$TEST_NUM  >>$LTPROOT/output/cpuctl_results_$FILE.txt &
		if [ $? -ne 0 ]
		then
			echo "Error: Could not run ./cpuctl_def_task01"
			cleanup;
			exit -1;
		else
			echo "Successfully launched def task $! too";
		fi
		;;
	"4" )
		if [ -f cpuctl_test02 ]
		then
		echo `date` >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo `uname -a` >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo TEST:- $TEST_NAME $TEST_NUM >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo NUM_GROUPS=$NUM_GROUPS +1 \(DEF\) >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		for i in $(seq 1 $NUM_GROUPS)
		do
			MYGROUP=/dev/cpuctl/group_$i
			TASKS_IN_GROUP=`expr $i \* 2`;
			for j in $(seq 1 $TASKS_IN_GROUP)
			do
			TASK_NUM=`expr $TASK_NUM + 1`;
			cp cpuctl_test02 cpuctl_task_$TASK_NUM ;
			chmod +x cpuctl_task_$TASK_NUM;
			if [ $i -eq 1 ]	# Renice all tasks of group 1
			then
				NICELEVEL=$NICEVALUE;
			else
				NICELEVEL=0;
			fi;

			GROUP_NUM=$i MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID NUM_CPUS=$NUM_CPUS \
			TEST_NUM=$TEST_NUM TASK_NUM=$TASK_NUM nice -n $NICELEVEL ./cpuctl_task_$TASK_NUM \
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
		if [ ! -f cpuctl_def_task02 ]; then
			echo "Source file for default task not compiled";
			echo "Plz check Makefile...Exiting test";
			cleanup;
			exit -1;
		fi
		MYGROUP=/dev/cpuctl/group_def ;
		GROUP_NUM=0 MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID \
		NUM_CPUS=$NUM_CPUS TEST_NUM=$TEST_NUM TASK_NUM=0 \
		./cpuctl_def_task02 >>$LTPROOT/output/cpuctl_results_$FILE.txt &
		if [ $? -ne 0 ]
		then
			echo "Error: Could not run ./cpuctl_def_task02"
			cleanup;
			exit -1;
		else
			echo "Successfully launched def task $! too";
		fi
		;;
	"5" )
		if [ -f cpuctl_test02 ]
		then
		echo `date` >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo `uname -a` >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo TEST:- $TEST_NAME $TEST_NUM >> $LTPROOT/output/cpuctl_results_$FILE.txt;
		echo NUM_GROUPS=$NUM_GROUPS +1 \(DEF\)>> $LTPROOT/output/cpuctl_results_$FILE.txt;
		TASKS_IN_GROUP=3;
		for i in $(seq 1 $NUM_GROUPS)
		do
			MYGROUP=/dev/cpuctl/group_$i
			for j in $(seq 1 $TASKS_IN_GROUP)
			do
			TASK_NUM=`expr $TASK_NUM + 1`;
			cp cpuctl_test02 cpuctl_task_$TASK_NUM ;
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
		if [ ! -f cpuctl_def_task02 ]; then
			echo "Source file for default task not compiled";
			echo "Plz check Makefile...Exiting test";
			cleanup;
			exit -1;
		fi
		MYGROUP=/dev/cpuctl/group_def ;
		GROUP_NUM=0 MYGROUP=$MYGROUP SCRIPT_PID=$SCRIPT_PID \
		NUM_CPUS=$NUM_CPUS TEST_NUM=$TEST_NUM TASK_NUM=0 \
		./cpuctl_def_task02 >>$LTPROOT/output/cpuctl_results_$FILE.txt &
		if [ $? -ne 0 ]
		then
			echo "Error: Could not run ./cpuctl_def_task02"
			cleanup;
			exit -1;
		else
			echo "Successfully launched def task $! too";
		fi
		;;

	 *  )
		echo "Could not start cpu controller test";
		echo "usage: run_cpuctl_test.sh test_num";
		echo "Skipping the test...";
		exit -1;;
	esac

	sleep 3
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
			echo "Task $i exited abnormally with return value: $RC";
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
