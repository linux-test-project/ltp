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
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        memcg_use_hierarchy_test.sh
# Description:  Tests memory.use_hierarchy.
# Author:       Peng Haitao <penght@cn.fujitsu.com>
# History:      2012/01/14 - Created.
#

export TCID="memcg_use_hierarchy_test"
export TST_TOTAL=3
export TST_COUNT=0

. ./memcg_lib.sh || exit 1

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
	echo 1 > memory.use_hierarchy 2> /dev/null
	result $(( !($? != 0) )) "return value is $?"

	rmdir subgroup
}

# test disabling will fail if the parent cgroup has enabled hierarchy.
testcase_3()
{
	echo 1 > memory.use_hierarchy
	mkdir subgroup
	echo 0 > subgroup/memory.use_hierarchy 2> /dev/null
	result $(( !($? != 0) )) "return value is $?"

	rmdir subgroup
}

# Run all the test cases
for i in $(seq 1 $TST_TOTAL)
do
	export TST_COUNT=$(( $TST_COUNT + 1 ))
	cur_id=$i

	do_mount
	if [ $? -ne 0 ]; then
		echo "Cannot create memcg"
		exit 1
	fi

	# prepare
	mkdir /dev/memcg/$i 2> /dev/null
	cd /dev/memcg/$i

	# run the case
	testcase_$i

	# clean up
	sleep 1
	cd $TEST_PATH
	rmdir /dev/memcg/$i

	cleanup
done

if [ $failed -ne 0 ]; then
	exit $failed
else
	exit 0
fi
