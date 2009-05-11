#!/bin/bash
# usage ./run_cpuset_test.sh test_num

################################################################################
#                                                                              #
# Copyright (c) 2009 FUJITSU LIMITED                                           #
#                                                                              #
# This program is free software;  you can redistribute it and#or modify        #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful, but          #
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY   #
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License     #
# for more details.                                                            #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program;  if not, write to the Free Software                 #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA      #
#                                                                              #
################################################################################
# Name Of File: run_cpuset_test.sh                                             #
#                                                                              #
# Description: This file runs the setup for testing different cpuset resource  #
#              controller features. After setup it runs diff test cases in     #
#              diff setup.                                                     #
#                                                                              #
# Test 01:     Tests basal operation of control file                           #
#                                                                              #
# Precaution:   Avoid system use by other applications/users to get fair and   #
#               appropriate results (avoid unnecessary killing of applicatio)  #
#                                                                              #
# Author:       Miao Xie   <miaox@cn.fujitsu.com>                              #
#                                                                              #
# History:                                                                     #
#                                                                              #
#  DATE         NAME           EMAIL                         DESC              #
#                                                                              #
#  02/03/09  Miao Xie     <miaox@cn.fujitsu.com>        Created this test      #
#                                                                              #
################################################################################

source cpuset_funcs.sh;

export TCID="cpuset_test";
export TST_TOTAL=1;
export TST_COUNT=1;

TEST_NUM=$1;
SCRIPT_PID=$$;
RC=0;
PWD=`pwd`;

check
if [ $? -ne 0 ]; then
	exit 0
fi

# The usage of the script file
usage()
{
	echo "Could not start cpuset controller test";
	echo "usage: run_cpuset_test.sh test_num";
	echo "Skipping the cpuset controller test...";
}

cd $LTPROOT/testcases/bin/

echo "TEST $TEST_NUM: CPUSET CONTROLLER TESTING";

echo "TEST STARTED: Please avoid using system while this test executes";
#Check if  C source  file has been compiled and then run it in different groups

case $TEST_NUM in
"1" )
	if [ -f cpuset_base_ops_testset.sh ]; then
		./cpuset_base_ops_testset.sh
		exit $?;
	else
		echo "Shell file not installed..Please check Makefile...Exiting"
		exit -1;
	fi;
	;;
"2" )
	if [ -f cpuset_inherit_testset.sh ]; then
		./cpuset_inherit_testset.sh
		exit $?;
	else
		echo "Shell file not installed..Please check Makefile...Exiting"
		exit -1;
	fi;
	;;
"3" )
	if [ -f cpuset_exclusive_test.sh ]; then
		./cpuset_exclusive_test.sh
		exit $?;
	else
		echo "Shell file not installed..Please check Makefile...Exiting"
		exit -1;
	fi;
	;;
"4" )
	if [ -f cpuset_hierarchy_test.sh ]; then
		./cpuset_hierarchy_test.sh
		exit $?;
	else
		echo "Shell file not installed..Please check Makefile...Exiting"
		exit -1;
	fi;
	;;
"5" )
	if [ -f cpuset_syscall_testset.sh ]; then
		./cpuset_syscall_testset.sh
		exit $?;
	else
		echo "Shell file not installed..Please check Makefile...Exiting"
		exit -1;
	fi;
	;;
"6" )
	if [ -f cpuset_sched_domains_test.sh ]; then
		./cpuset_sched_domains_test.sh
		exit $?;
	else
		echo "Shell file not installed..Please check Makefile...Exiting"
		exit -1;
	fi;
	;;
"7" )
	if [ -f cpuset_load_balance_test.sh ]; then
		./cpuset_load_balance_test.sh
		exit $?;
	else
		echo "Shell file not installed..Please check Makefile...Exiting"
		exit -1;
	fi;
	;;
"8" )
	if [ -f cpuset_hotplug_test.sh ]; then
		./cpuset_hotplug_test.sh
		exit $?;
	else
		echo "Shell file not installed..Please check Makefile...Exiting"
		exit -1;
	fi;
	;;
"9" )
	if [ -f cpuset_memory_testset.sh ]; then
		./cpuset_memory_testset.sh
		exit $?;
	else
		echo "Shell file not installed..Please check Makefile...Exiting"
		exit -1;
	fi;
	;;
"10" )
	if [ -f cpuset_memory_pressure_testset.sh ]; then
		./cpuset_memory_pressure_testset.sh
		exit $?;
	else
		echo "Shell file not installed..Please check Makefile...Exiting"
		exit -1;
	fi;
	;;
"11" )
	if [ -f cpuset_memory_spread_testset.sh ]; then
		./cpuset_memory_spread_testset.sh
		exit $?;
	else
		echo "Shell file not installed..Please check Makefile...Exiting"
		exit -1;
	fi;
	;;
"*" )
	usage;
	exit -1;
	;;
esac

echo "Cpuset Resource Controller test executed successfully.";
cd $PWD
exit 0;         #to let PAN reprt success of test
