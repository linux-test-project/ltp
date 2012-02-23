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
# File :        memcg_stat_test.sh
# Description:  Tests memroy.stat.
# Author:       Peng Haitao <penght@cn.fujitsu.com>
# History:      2012/01/16 - Created.
#

export TCID="memcg_stat_test"
export TST_TOTAL=8
export TST_COUNT=0

. ./memcg_lib.sh || exit 1

MEM_SWAP_FLAG=0

# Test cache
testcase_1()
{
	test_mem_stat "--shm -k 3" $PAGESIZE "cache" $PAGESIZE 0
}

# Test mapped_file
testcase_2()
{
	test_mem_stat "--mmap-file" $PAGESIZE "mapped_file" $PAGESIZE 0
}

# Test unevictable with MAP_LOCKED
testcase_3()
{
	test_mem_stat "--mmap-lock1" $PAGESIZE "unevictable" $PAGESIZE 0
}

# Test unevictable with mlock
testcase_4()
{
	test_mem_stat "--mmap-lock2" $PAGESIZE "unevictable" $PAGESIZE 0
}

# Test hierarchical_memory_limit with enabling hierarchical accounting
testcase_5()
{
	echo 1 > memory.use_hierarchy

	mkdir subgroup
	echo $PAGESIZE > memory.limit_in_bytes
	echo $((PAGESIZE*2)) > subgroup/memory.limit_in_bytes

	cd subgroup
	check_mem_stat "hierarchical_memory_limit" $PAGESIZE

	cd ..
	rmdir subgroup
}

# Test hierarchical_memory_limit with disabling hierarchical accounting
testcase_6()
{
	echo 0 > memory.use_hierarchy

	mkdir subgroup
	echo $PAGESIZE > memory.limit_in_bytes
	echo $((PAGESIZE*2)) > subgroup/memory.limit_in_bytes

	cd subgroup
	check_mem_stat "hierarchical_memory_limit" $((PAGESIZE*2))

	cd ..
	rmdir subgroup
}

# Test hierarchical_memsw_limit with enabling hierarchical accounting
testcase_7()
{
	if [ $MEM_SWAP_FLAG -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo 1 > memory.use_hierarchy

	mkdir subgroup
	echo $PAGESIZE > memory.limit_in_bytes
	echo $PAGESIZE > memory.memsw.limit_in_bytes
	echo $((PAGESIZE*2)) > subgroup/memory.limit_in_bytes
	echo $((PAGESIZE*2)) > subgroup/memory.memsw.limit_in_bytes

	cd subgroup
	check_mem_stat "hierarchical_memsw_limit" $PAGESIZE

	cd ..
	rmdir subgroup
}

# Test hierarchical_memsw_limit with disabling hierarchical accounting
testcase_8()
{
	if [ $MEM_SWAP_FLAG -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo 0 > memory.use_hierarchy

	mkdir subgroup
	echo $PAGESIZE > memory.limit_in_bytes
	echo $PAGESIZE > memory.memsw.limit_in_bytes
	echo $((PAGESIZE*2)) > subgroup/memory.limit_in_bytes
	echo $((PAGESIZE*2)) > subgroup/memory.memsw.limit_in_bytes

	cd subgroup
	check_mem_stat "hierarchical_memsw_limit" $((PAGESIZE*2))

	cd ..
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

	if [ -e memory.memsw.limit_in_bytes ]; then
		MEM_SWAP_FLAG=1
	fi

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
