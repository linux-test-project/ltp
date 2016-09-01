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

TCID="memcg_failcnt"
TST_TOTAL=3

. memcg_lib.sh

# Test memory.failcnt
testcase_1()
{
	echo $PAGESIZE > memory.limit_in_bytes
	malloc_free_memory "--mmap-anon" $(($PAGESIZE*2))
	test_failcnt "memory.failcnt"
}

testcase_2()
{
	echo $PAGESIZE > memory.limit_in_bytes
	malloc_free_memory "--mmap-file" $(($PAGESIZE*2))
	test_failcnt "memory.failcnt"
}

testcase_3()
{
	echo $PAGESIZE > memory.limit_in_bytes
	malloc_free_memory "--shm" $(($PAGESIZE*2))
	test_failcnt "memory.failcnt"
}

shmmax_setup
LOCAL_CLEANUP=shmmax_cleanup
run_tests
tst_exit
