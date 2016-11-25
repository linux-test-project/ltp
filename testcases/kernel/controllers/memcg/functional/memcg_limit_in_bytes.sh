#!/bin/sh

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
## along with this program;  if not, write to the Free Software Foundation,   ##
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           ##
##                                                                            ##
## Author: Li Zefan <lizf@cn.fujitsu.com>                                     ##
## Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>                     ##
## Added memcg enable/disable functinality: Rishikesh K Rajak		      ##
##						<risrajak@linux.vnet.ibm.com  ##
##                                                                            ##
################################################################################

TCID="memcg_limit_in_bytes"
TST_TOTAL=15

. memcg_lib.sh

# Test mmap(locked) + alloc_mem > limit_in_bytes
testcase_1()
{
	test_proc_kill $PAGESIZE "--mmap-lock1" $((PAGESIZE*2)) 0
}

testcase_2()
{
	test_proc_kill $PAGESIZE "--mmap-lock2" $((PAGESIZE*2)) 0
}

# Test swapoff + alloc_mem > limit_in_bytes
testcase_3()
{
	swapoff -a
	test_proc_kill $PAGESIZE "--mmap-anon" $((PAGESIZE*2)) 0
	swapon -a
}

testcase_4()
{
	swapoff -a
	test_proc_kill $PAGESIZE "--mmap-file" $((PAGESIZE*2)) 0
	swapon -a
}

testcase_5()
{
	swapoff -a
	test_proc_kill $PAGESIZE "--shm -k 18" $((PAGESIZE*2)) 0
	swapon -a
}

# Test limit_in_bytes == 0
testcase_6()
{
	test_proc_kill 0 "--mmap-anon" $PAGESIZE 0
}

testcase_7()
{
	test_proc_kill 0 "--mmap-file" $PAGESIZE 0
}

testcase_8()
{
	test_proc_kill 0 "--shm -k 21" $PAGESIZE 0
}

# Test limit_in_bytes will be aligned to PAGESIZE
testcase_9()
{
	test_limit_in_bytes $((PAGESIZE-1)) 0
}

testcase_10()
{
	test_limit_in_bytes $((PAGESIZE+1)) 0
}

testcase_11()
{
	test_limit_in_bytes 1 0
}

# Test invalid memory.limit_in_bytes
testcase_12()
{
	if tst_kvcmp -lt "2.6.31"; then
		EXPECT_FAIL echo -1 \> memory.limit_in_bytes
	else
		EXPECT_PASS echo -1 \> memory.limit_in_bytes
	fi
}

testcase_13()
{
	EXPECT_FAIL echo 1.0 \> memory.limit_in_bytes
}

testcase_14()
{
	EXPECT_FAIL echo 1xx \> memory.limit_in_bytes
}

testcase_15()
{
	EXPECT_FAIL echo xx \> memory.limit_in_bytes
}

shmmax_setup
LOCAL_CLEANUP=shmmax_cleanup
run_tests
tst_exit
