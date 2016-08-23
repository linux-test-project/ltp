#! /bin/sh

################################################################################
##                                                                            ##
## Copyright (c) 2012 FUJITSU LIMITED                                         ##
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
################################################################################
#
# File :        memcg_use_hierarchy_test.sh
# Description:  Tests memory.use_hierarchy.
# Author:       Peng Haitao <penght@cn.fujitsu.com>
# History:      2012/01/14 - Created.
#

TCID="memcg_use_hierarchy_test"
TST_TOTAL=3

. memcg_lib.sh

# test if one of the ancestors goes over its limit, the proces will be killed
testcase_1()
{
	echo 1 > memory.use_hierarchy
	echo $PAGESIZE > memory.limit_in_bytes

	mkdir subgroup
	cd subgroup
	test_proc_kill $((PAGESIZE*3)) "--mmap-lock1" $((PAGESIZE*2)) 0

	cd ..
	rmdir subgroup
}

# test Enabling will fail if the cgroup already has other cgroups
testcase_2()
{
	mkdir subgroup
	EXPECT_FAIL echo 1 \> memory.use_hierarchy

	rmdir subgroup
}

# test disabling will fail if the parent cgroup has enabled hierarchy.
testcase_3()
{
	echo 1 > memory.use_hierarchy
	mkdir subgroup
	EXPECT_FAIL echo 0 \> subgroup/memory.use_hierarchy

	rmdir subgroup
}

run_tests

tst_exit

