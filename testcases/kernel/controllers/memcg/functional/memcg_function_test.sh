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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
## Author: Li Zefan <lizf@cn.fujitsu.com>                                     ##
## Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>                     ##
## Added memcg enable/disable functinality: Rishikesh K Rajak		      ##
##						<risrajak@linux.vnet.ibm.com  ##
##                                                                            ##
################################################################################

TCID="memcg_function_test"
TST_TOTAL=38

shmmax_cleanup()
{
	if [ -n "$shmmax" ]; then
		echo "$shmmax" > /proc/sys/kernel/shmmax
	fi
}
LOCAL_CLEANUP=shmmax_cleanup

. memcg_lib.sh

# Case 1 - 10: Test the management and counting of memory
testcase_1()
{
	test_mem_stat "--mmap-anon" $PAGESIZE "rss" $PAGESIZE 0
}

testcase_2()
{
	test_mem_stat "--mmap-file" $PAGESIZE "rss" 0 0
}

testcase_3()
{
	test_mem_stat "--shm -k 3" $PAGESIZE "rss" 0 0
}

testcase_4()
{
	test_mem_stat "--mmap-anon --mmap-file --shm" $PAGESIZE "rss" \
		$PAGESIZE 0
}

testcase_5()
{
	test_mem_stat "--mmap-lock1" $PAGESIZE "rss" $PAGESIZE 0
}

testcase_6()
{
	test_mem_stat "--mmap-anon" $PAGESIZE "rss" $PAGESIZE 1
}

testcase_7()
{
	test_mem_stat "--mmap-file" $PAGESIZE "rss" 0 1
}

testcase_8()
{
	test_mem_stat "--shm -k 8" $PAGESIZE "rss" 0 1
}

testcase_9()
{
	test_mem_stat "--mmap-anon --mmap-file --shm" $PAGESIZE "rss" \
		$PAGESIZE 1
}

testcase_10()
{
	test_mem_stat "--mmap-lock1" $PAGESIZE "rss" $PAGESIZE 1
}

# Case 11 - 13: Test memory.failcnt
testcase_11()
{
	echo $PAGESIZE > memory.limit_in_bytes
	malloc_free_memory "--mmap-anon" $(($PAGESIZE*2))
	test_failcnt "memory.failcnt"
}

testcase_12()
{
	echo $PAGESIZE > memory.limit_in_bytes
	malloc_free_memory "--mmap-file" $(($PAGESIZE*2))
	test_failcnt "memory.failcnt"
}

testcase_13()
{
	echo $PAGESIZE > memory.limit_in_bytes
	malloc_free_memory "--shm" $(($PAGESIZE*2))
	test_failcnt "memory.failcnt"
}

# Case 14 - 15: Test mmap(locked) + alloc_mem > limit_in_bytes
testcase_14()
{
	test_proc_kill $PAGESIZE "--mmap-lock1" $((PAGESIZE*2)) 0
}

testcase_15()
{
	test_proc_kill $PAGESIZE "--mmap-lock2" $((PAGESIZE*2)) 0
}

# Case 16 - 18: Test swapoff + alloc_mem > limi_in_bytes
testcase_16()
{
	swapoff -a
	test_proc_kill $PAGESIZE "--mmap-anon" $((PAGESIZE*2)) 0
	swapon -a
}

testcase_17()
{
	swapoff -a
	test_proc_kill $PAGESIZE "--mmap-file" $((PAGESIZE*2)) 0
	swapon -a
}

testcase_18()
{
	swapoff -a
	test_proc_kill $PAGESIZE "--shm -k 18" $((PAGESIZE*2)) 0
	swapon -a
}

# Case 19 - 21: Test limit_in_bytes == 0
testcase_19()
{
	test_proc_kill 0 "--mmap-anon" $PAGESIZE 0
}

testcase_20()
{
	test_proc_kill 0 "--mmap-file" $PAGESIZE 0
}

testcase_21()
{
	test_proc_kill 0 "--shm -k 21" $PAGESIZE 0
}

# Case 22 - 24: Test limit_in_bytes will be aligned to PAGESIZE
testcase_22()
{
	test_limit_in_bytes $((PAGESIZE-1)) 0
}

testcase_23()
{
	test_limit_in_bytes $((PAGESIZE+1)) 0
}

testcase_24()
{
	test_limit_in_bytes 1 0
}

# Case 25 - 28: Test invaild memory.limit_in_bytes
testcase_25()
{
	tst_kvercmp 2 6 31
	if [ $? -eq 0 ]; then
		EXPECT_FAIL echo -1 \> memory.limit_in_bytes
	else
		EXPECT_PASS echo -1 \> memory.limit_in_bytes
	fi
}

testcase_26()
{
	EXPECT_FAIL echo 1.0 \> memory.limit_in_bytes
}

testcase_27()
{
	EXPECT_FAIL echo 1xx \> memory.limit_in_bytes
}

testcase_28()
{
	EXPECT_FAIL echo xx \> memory.limit_in_bytes
}

# Case 29 - 35: Test memory.force_empty
testcase_29()
{
	memcg_process --mmap-anon -s $PAGESIZE &
	pid=$!
	TST_CHECKPOINT_WAIT 0
	echo $pid > tasks
	kill -s USR1 $pid 2> /dev/null
	sleep 1
	echo $pid > ../tasks

	# This expects that there is swap configured
	EXPECT_PASS echo 1 \> memory.force_empty

	kill -s INT $pid 2> /dev/null
}

testcase_30()
{
	memcg_process --mmap-lock2 -s $PAGESIZE &
	pid=$!
	TST_CHECKPOINT_WAIT 0
	echo $pid > tasks
	kill -s USR1 $pid 2> /dev/null
	sleep 1

	EXPECT_FAIL echo 1 \> memory.force_empty

	kill -s INT $pid 2> /dev/null
}

testcase_31()
{
	EXPECT_PASS echo 0 \> memory.force_empty
}

testcase_32()
{
	EXPECT_PASS echo 1.0 \> memory.force_empty
}

testcase_33()
{
	EXPECT_PASS echo 1xx \> memory.force_empty
}

testcase_34()
{
	EXPECT_PASS echo xx \> memory.force_empty
}

testcase_35()
{
	# writing to non-empty top mem cgroup's force_empty
	# should return failure
	EXPECT_FAIL echo 1 \> /dev/memcg/memory.force_empty
}

# Case 36 - 38: Test that group and subgroup have no relationship
testcase_36()
{
	test_subgroup $PAGESIZE $((2*PAGESIZE))
}

testcase_37()
{
	test_subgroup $PAGESIZE $PAGESIZE
}

testcase_38()
{
	test_subgroup $PAGESIZE 0
}

shmmax=`cat /proc/sys/kernel/shmmax`
if [ $shmmax -lt $HUGEPAGESIZE ]; then
	ROD echo "$HUGEPAGESIZE" \> /proc/sys/kernel/shmmax
fi

run_tests

tst_exit
