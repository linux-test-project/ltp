#!/bin/bash
# usage ./run_cpuacct_test.sh $TEST_NUM
#############################################################################
#  Copyright (c) International Business Machines  Corp., 2009               #
#                                                                           #
#  This program is free software;  you can redistribute it and/or modify    #
#  it under the terms of the GNU General Public License as published by     #
#  the Free Software Foundation; either version 2 of the License, or        #
#  (at your option) any later version.                                      #
#                                                                           #
#  This program is distributed in the hope that it will be useful,          #
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of          #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                #
#  the GNU General Public License for more details.                         #
#                                                                           #
#  You should have received a copy of the GNU General Public License        #
#  along with this program;  if not, write to the Free Software             #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  #
#                                                                           #
#############################################################################
# Name Of File: run_cpuacct_test.sh                                         #
#                                                                           #
# Description: This file runs the setup for testing different cpu acctount  #
#              controller features. After setup it runs diff test cases in  #
#		diff setup.                                                 #
#                                                                           #
# Test 01:     Tests Cpu usage of Hierarchical cgroups                      #
#                                                                           #
# Precaution:   Avoid system use by other applications/users to get fair and#
#               appropriate results (avoid unnecessary killing of 	    #
#		application)						    #
#                                                                           #
# Author:       Rajasekhar Duddu   <rajduddu@in.ibm.com>                    #
#                                                                           #
# History:                                                                  #
#                                                                           #
#  DATE         NAME           EMAIL                         DESC           #
#                                                                           #
#  14/07/09  Rajasekhar D    <rajduddu@in.ibm.com>        Created this test #
#                                                                           #
#############################################################################

export TCID="cpuacct_test01";
export TST_TOTAL=1;
export TST_COUNT=1;

TEST_NUM=$1;
SCRIPT_PID=$$;
RC=0;
PWD=`pwd`;

cd $LTPROOT/testcases/bin/  2> /dev/null
. cpuacct_setup.sh

if [ "$USER" != root ]; then
        tst_brkm TBROK ignored "Test must be run as root"
        exit 0
fi

tst_kvercmp 2 6 30  2> /dev/null
if [ $? -eq 0 ]; then
        tst_brkm TBROK ignored "Test should be run with kernel 2.6.30 or newer"
        exit 0
fi

task_kill  2> /dev/null
cleanup

mes="CPU Accounting Controller"
cg_path="/dev/cpuacct";
num_online_cpus=`cat /proc/cpuinfo | grep -w -i processor | wc -l`

#Function to create tasks equal to num_online_cpus.
nr_tasks ()
{
	$PWD/cpuacct_task01 &
	pid=$!
}

#Function to caluculate the threshold value.
get_threshold ()
{
	num_online_cpus=`expr $num_online_cpus \* $num_online_cpus`
	if [ $num_online_cpus -le 32 ]
	then
		threshold=32
	else
		threshold=$num_online_cpus
	fi
	threshold=`expr $threshold \* 2`
}

#Function which is called for reading the cpuacct.usage_percpu stat value
#for Parent and Child cgroups.
per_cpu_usage ()
{
	attrc=0
	attrp=0
        i=0
        k=0
        while read line
        do
	        j=0
                for k in $line
                do
        	        j=`expr $j + $k`
                done
                if [ "$i" == "0" ]
                then
   	             attrp=$j
                     i=`expr $i + 1`
                else
                     attrc=`expr $j + $attrc`
                fi
        done < "./tmp2"
}

#Function which verifies the cpu accounting of the Parent and the Child cgroups.
check_attr()
{

	if [ "$1" == "1" ]
	then
		if [ "$2" == "cpuacct.stat" ]
		then
			attr1="`sed -n 1p tmp2`"
			attr2="`sed -n 2p tmp2`"
			attr3="`sed -n 3p tmp2`"
			attr4="`sed -n 4p tmp2`"
			echo
			echo "$2 for Parent cgroup is $attr1 : $attr2"
			echo "$2 for Child cgroup is $attr3 : $attr4"

		        if [ "$attr1" == "$attr3" ] && [ "$attr2" == "$attr4" ]
		        then
				RC=$?
				echo "TPASS $mes:$2 PASSED"

		        else
				RC=$?
				echo "TFAIL $mes:$2 FAILED"
		        fi
		elif [ "$2" == "cpuacct.usage_percpu" ]
                then
			per_cpu_usage
			echo
                        echo "$2 for Parent cgroup : $attrp"
                        echo "$2 for Child  cgroup : $attrc"
                        if [ "$attrp" == "$attrc" ]
                        then
                                RC=$?
                                echo "TPASS $mes:$2 PASSED"
                        else
                                RC=$?
                                echo "TFAIL $mes:$2 FAILED"
                        fi
		else
			attr1="`sed -n 1p tmp2`"
			attr2="`sed -n 2p tmp2`"

			echo
			echo "$2 for Parent cgroup is $attr1"
			echo "$2 for Child cgroup is $attr2"
		        if [ "$attr1"  ==   "$attr2"  ]
		        then
				RC=$?
			        echo "TPASS $mes:$2 PASSED"
			else
				RC=$?
				echo "TFAIL $mes:$2 FAILED"
			fi

		fi
	else

		if [ "$2" == "cpuacct.stat" ]
		then
			attr0="`sed -n 1p tmp2 | cut -d" " -f2`"
			attr1="`sed -n 2p tmp2 | cut -d" " -f2`"
			attr2="`sed -n 3p tmp2 | cut -d" " -f2`"
			attr3="`sed -n 4p tmp2 | cut -d" " -f2`"
			attr4="`sed -n 5p tmp2 | cut -d" " -f2`"
			attr5="`sed -n 6p tmp2 | cut -d" " -f2`"
			attr_usr=`expr $attr2 + $attr4 `
			attr_sys=`expr $attr3 + $attr5`
			echo
			echo "$2 for Parent cgroup : $attr0::$attr1"
			echo "$2 for Child  cgroup : $attr_usr::$attr_sys"
			get_threshold
			diff_usr=`expr $attr0 - $attr_usr `
			[ ${diff_usr} -le 0 ] &&  diff_usr=$((0 - $diff_usr))

			diff_sys=`expr $attr1 - $attr_sys`
			[ ${diff_sys} -le 0 ] &&  diff_sys=$((0 - $diff_sys))
			if [ "$diff_usr" -le "$threshold" ] && \
			[ "$diff_sys" -le "$threshold" ]
			then
				RC=$?
			        echo "TPASS $mes:$2 PASSED"
		        else
				RC=$?
			        echo "TFAIL $mes:$2 FAILED"
			fi
		elif [ "$2" == "cpuacct.usage_percpu" ]
		then
			per_cpu_usage
			echo
			echo "$2 for Parent cgroup : $attrp"
			echo "$2 for Child  cgroup : $attrc"
			if [ "$attrp" == "$attrc" ]
			then
				RC=$?
			        echo "TPASS $mes:$2 PASSED"
		        else
				RC=$?
			        echo "TFAIL $mes:$2 FAILED"
		        fi

		else
			attr0="`sed -n 1p tmp2`"
			attr1="`sed -n 2p tmp2`"
			attr2="`sed -n 3p tmp2`"
			attr=`expr $attr1 + $attr2`
			echo
			echo "$2 for Parent cgroup : $attr0"
			echo "$2 for Child  cgroup : $attr"
			if [ "$attr0" == "$attr" ]
			then
				RC=$?
			        echo "TPASS $mes:$2 PASSED"
		        else
				RC=$?
			        echo "TFAIL $mes:$2 FAILED"
		        fi
		fi
	fi
}

echo "TEST $TEST_NUM:CPU ACCOUNTING CONTROLLER TESTING";
echo "RUNNING SETUP.....";
setup;

echo "TEST STARTED: Please avoid using system while this test executes";


status=0
case ${TEST_NUM} in
	"1" )
		ls $PWD/cpuacct_task01 &> /dev/null
		if [ $? -ne 0 ]
		then
		        echo "TFAIL Task file cpuacct_task01.c not compiled"
			echo "Please check Makefile Exiting test"
			task_kill 2> /dev/null
		        exit -1
		fi
		$PWD/cpuacct_task01 &
                pid=$!

		mkdir $cg_path/group_1 2> /dev/null
		mkdir $cg_path/group_1/group_11/ 2> /dev/null
		if [ $? -ne 0 ]
                then
                        echo "TFAIL Cannot create cpuacct cgroups Exiting Test"
                        cleanup
			task_kill 2> /dev/null
                        exit -1
                fi
		echo $pid > /$cg_path/group_1/group_11/tasks 2> /dev/null
		if [ $? -ne 0 ]
		then
			echo "TFAIL Not able to move a task to the cgroup"
			echo "Exiting Test"
			cleanup 2> /dev/null
			task_kill 2> /dev/null
			exit -1
		fi
		sleep 5
		task_kill 2> /dev/null
		for i in cpuacct.usage cpuacct.usage_percpu cpuacct.stat
		do
			cat $cg_path/group_1/$i \
			$cg_path/group_1/group_11/$i > tmp2
			check_attr $1 $i
		if [ $RC -ne 0 ]
		then
			status=1
		fi
		done
		if [ $status -eq 0 ]
		then
			echo
			echo "$mes test executed successfully"
			cleanup 2> /dev/null
			task_kill 2> /dev/null
			exit 0
		else
			echo
			echo "$mes test execution Failed"
			cleanup 2> /dev/null
			exit -1
		fi
		;;

	"2" )
		mkdir $cg_path/group_1 2> /dev/null
		mkdir $cg_path/group_1/group_11 2> /dev/null
		mkdir $cg_path/group_1/group_12 2> /dev/null
                if [ $? -ne 0 ]
                then
          		echo "TFAIL Cannot create cpuacct cgroups Exiting Test"
                        cleanup 2> /dev/null
			task_kill 2> /dev/null
                        exit -1
                fi

                ls $PWD/cpuacct_task01 &> /dev/null
                if [ $? -ne 0 ]
                then
                        echo "TFAIL Task file cpuacct_task01.c not compiled"
			echo "Please check Makefile Exiting test"
                	cleanup 2> /dev/null
			task_kill 2> /dev/null
		        exit -1
                fi
		for (( m=0 ; m<=$num_online_cpus ; m++ ))
		do
			nr_tasks
        	        echo $pid > $cg_path/group_1/group_11/tasks
			if [ $? -ne 0 ]
                	then
                        	echo "TFAIL Not able to move task to cgroup"
				echo "Exiting Test"
	                        cleanup 2> /dev/null
				task_kill 2> /dev/null
				exit -1
        	        fi
			nr_tasks
			echo $pid >$cg_path/group_1/group_12/tasks
	                if [ $? -ne 0 ]
        	        then
                	        echo "TFAIL Not able to move task to cgroup"
				echo "Exiting Test"
				cleanup 2> /dev/null
				task_kill 2> /dev/null
                        	exit -1
	                fi
			sleep 2
		done
		task_kill 2> /dev/null
		for i in cpuacct.usage cpuacct.usage_percpu cpuacct.stat
                do
                        cat $cg_path/group_1/$i  \
			$cg_path/group_1/group_11/$i \
			$cg_path/group_1/group_12/$i >tmp2
                        check_attr $1 $i
		if [ $RC -ne 0 ]
                then
                        status=1
                fi
		done
		if [ $status -eq 0 ]
                then
                        echo
                        echo "$mes test executed successfully"
                        cleanup 2> /dev/null
			task_kill 2> /dev/null
			cd $PWD
                        exit 0
                else
                        echo
                        echo "$mes test execution Failed"
                        cleanup 2> /dev/null
			task_kill 2> /dev/null
			cd $PWD
                        exit -1
                fi

		;;
	* )
		usage
		exit -1
		;;
	esac

