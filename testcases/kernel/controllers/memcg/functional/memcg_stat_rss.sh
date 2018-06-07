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

TCID="memcg_stat_rss"
TST_TOTAL=10

. memcg_lib.sh

# Test the management and counting of memory
testcase_1()
{
	test_mem_stat "--mmap-anon" $PAGESIZES $PAGESIZES "rss" $PAGESIZES false
}

testcase_2()
{
	test_mem_stat "--mmap-file" $PAGESIZE $PAGESIZE "rss" 0 false
}

testcase_3()
{
	test_mem_stat "--shm -k 3" $PAGESIZE $PAGESIZE "rss" 0 false
}

testcase_4()
{
	test_mem_stat "--mmap-anon --mmap-file --shm" \
		$PAGESIZES $((PAGESIZES*3)) "rss" $PAGESIZES false
}

testcase_5()
{
	test_mem_stat "--mmap-lock1" $PAGESIZES $PAGESIZES "rss" $PAGESIZES false
}

testcase_6()
{
	test_mem_stat "--mmap-anon" $PAGESIZES $PAGESIZES "rss" $PAGESIZES true
}

testcase_7()
{
	test_mem_stat "--mmap-file" $PAGESIZE $PAGESIZE "rss" 0 true
}

testcase_8()
{
	test_mem_stat "--shm -k 8" $PAGESIZE $PAGESIZE "rss" 0 true
}

testcase_9()
{
	test_mem_stat "--mmap-anon --mmap-file --shm" \
		$PAGESIZES $((PAGESIZES*3)) "rss" $PAGESIZES true
}

testcase_10()
{
	test_mem_stat "--mmap-lock1" $PAGESIZES $PAGESIZES "rss" $PAGESIZES true
}

shmmax_setup
LOCAL_CLEANUP=shmmax_cleanup
run_tests
tst_exit
