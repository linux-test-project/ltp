#!/bin/sh

################################################################################
##                                                                            ##
##   Copyright (c) 2010 Mohamed Naufal Basheer                                ##
##                                                                            ##
##   This program is free software;  you can redistribute it and/or modify    ##
##   it under the terms of the GNU General Public License as published by     ##
##   the Free Software Foundation; either version 2 of the License, or        ##
##   (at your option) any later version.                                      ##
##                                                                            ##
##   This program is distributed in the hope that it will be useful,          ##
##   but WITHOUT ANY WARRANTY;  without even the implied warranty of          ##
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                ##
##   the GNU General Public License for more details.                         ##
##                                                                            ##
##   You should have received a copy of the GNU General Public License        ##
##   along with this program;  if not, write to the Free Software             ##
##   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA  ##
##                                                                            ##
##                                                                            ##
##   File:    memcg_control_test.sh                                           ##
##                                                                            ##
##   Purpose: Implement various memory controller tests                       ##
##                                                                            ##
##   Author:  Mohamed Naufal Basheer <naufal11@gmail.com>                     ##
##                                                                            ##
################################################################################

if [ "x$(grep -w memory /proc/cgroups | cut -f4)" != "x1" ]; then
	echo "WARNING:"
	echo "Either kernel does not support memory resource controller or feature not enabled"
	echo "Skipping all memcg_control testcases...."
	exit 0
fi

export TCID="memcg_control"
export TST_TOTAL=1
export TST_COUNT=0

export TMP=${TMP:-/tmp}
cd $TMP

PAGE_SIZE=$(tst_getconf PAGESIZE)

TOT_MEM_LIMIT=$PAGE_SIZE
ACTIVE_MEM_LIMIT=$PAGE_SIZE
PROC_MEM=$((PAGE_SIZE * 2))

TST_PATH=$PWD
STATUS_PIPE="$TMP/status_pipe"

PASS=0
FAIL=1

# Check if the test process is killed on crossing boundary
test_proc_kill()
{
	cd $TMP
	mem_process -m $PROC_MEM &
	cd $OLDPWD
	sleep 1
	echo $! > tasks

	#Instruct the test process to start acquiring memory
	echo m > $STATUS_PIPE
	sleep 5

	#Check if killed
	ps -p $! > /dev/null 2> /dev/null
	if [ $? -eq 0 ]; then
		echo m > $STATUS_PIPE
		echo x > $STATUS_PIPE
	else
		: $((KILLED_CNT += 1))
	fi
}

# Validate the memory usage limit imposed by the hierarchically topmost group
testcase_1()
{
	TST_COUNT=1
	tst_resm TINFO "Test #1: Checking if the memory usage limit imposed by the topmost group is enforced"

	echo "$ACTIVE_MEM_LIMIT" > $TST_PATH/mnt/$TST_NUM/memory.limit_in_bytes
	echo "$TOT_MEM_LIMIT" > $TST_PATH/mnt/$TST_NUM/memory.memsw.limit_in_bytes

	mkdir sub
	(cd sub
	KILLED_CNT=0
	test_proc_kill

	if [ $PROC_MEM -gt $TOT_MEM_LIMIT ] && [ $KILLED_CNT -eq 0 ]; then
		result $FAIL "Test #1: failed"
	else
		result $PASS "Test #1: passed"
	fi)
	rmdir sub
}

# Record the test results
#
# $1: Result of the test case, $PASS or $FAIL
# $2: Output information
result()
{
	RES=$1
	INFO=$2

	if [ $RES -eq $PASS ]; then
		tst_resm TPASS "$INFO"
	else
		: $((FAILED_CNT += 1))
		tst_resm TFAIL "$INFO"
	fi
}

cleanup()
{
	if [ -e $TST_PATH/mnt ]; then
		umount $TST_PATH/mnt 2> /dev/null
		rm -rf $TST_PATH/mnt
	fi
}

do_mount()
{
	cleanup

	mkdir $TST_PATH/mnt
	mount -t cgroup -o memory cgroup $TST_PATH/mnt 2> /dev/null
	if [ $? -ne 0 ]; then
		tst_brkm TBROK NULL "Mounting cgroup to temp dir failed"
		rmdir $TST_PATH/mnt
		exit 1
	fi
}

do_mount

echo 1 > mnt/memory.use_hierarchy 2> /dev/null

FAILED_CNT=0

TST_NUM=1
while [ $TST_NUM -le $TST_TOTAL ]; do
	mkdir $TST_PATH/mnt/$TST_NUM
	(cd $TST_PATH/mnt/$TST_NUM && testcase_$TST_NUM)
	rmdir $TST_PATH/mnt/$TST_NUM
	: $((TST_NUM += 1))
done

echo 0 > mnt/memory.use_hierarchy 2> /dev/null

cleanup

if [ "$FAILED_CNT" -ne 0 ]; then
	tst_resm TFAIL "memcg_control: failed"
	exit 1
else
	tst_resm TPASS "memcg_control: passed"
	exit 0
fi
