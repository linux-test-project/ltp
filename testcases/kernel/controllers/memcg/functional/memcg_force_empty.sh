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

TCID="memcg_force_empty"
TST_TOTAL=6

. memcg_lib.sh

# Test memory.force_empty
testcase_1()
{
	memcg_process --mmap-anon -s $PAGESIZE &
	pid=$!
	TST_CHECKPOINT_WAIT 0
	echo $pid > tasks
	signal_memcg_process $pid $PAGESIZE
	echo $pid > ../tasks

	# This expects that there is swap configured
	EXPECT_PASS echo 1 \> memory.force_empty

	stop_memcg_process $pid
}

testcase_2()
{
	EXPECT_PASS echo 0 \> memory.force_empty
}

testcase_3()
{
	EXPECT_PASS echo 1.0 \> memory.force_empty
}

testcase_4()
{
	EXPECT_PASS echo 1xx \> memory.force_empty
}

testcase_5()
{
	EXPECT_PASS echo xx \> memory.force_empty
}

testcase_6()
{
	# writing to non-empty top mem cgroup's force_empty
	# should return failure
	EXPECT_FAIL echo 1 \> /dev/memcg/memory.force_empty
}

run_tests
tst_exit
