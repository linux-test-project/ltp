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
# File :        memcg_max_usage_in_bytes_test.sh
# Description:  Tests memory.max_usage_in_bytes.
# Author:       Peng Haitao <penght@cn.fujitsu.com>
# History:      2012/01/17 - Created.
#

export TCID="memcg_max_usage_in_bytes_test"
export TST_TOTAL=4
export TST_COUNT=0

. ./memcg_lib.sh || exit 1

MEM_SWAP_FLAG=0

# Test memory.max_usage_in_bytes
testcase_1()
{
	test_max_usage_in_bytes "--mmap-anon" $((PAGESIZE*1024)) \
		"memory.max_usage_in_bytes" $((PAGESIZE*1024)) 0
}

# Test memory.memsw.max_usage_in_bytes
testcase_2()
{
	if [ $MEM_SWAP_FLAG -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo $((PAGESIZE*2048)) > memory.limit_in_bytes
	echo $((PAGESIZE*2048)) > memory.memsw.limit_in_bytes
	test_max_usage_in_bytes "--mmap-anon" $((PAGESIZE*1024)) \
		"memory.memsw.max_usage_in_bytes" $((PAGESIZE*1024)) 0
}

# Test reset memory.max_usage_in_bytes
testcase_3()
{
	test_max_usage_in_bytes "--mmap-anon" $((PAGESIZE*1024)) \
		"memory.max_usage_in_bytes" $((PAGESIZE*1024)) 1
}

# Test reset memory.memsw.max_usage_in_bytes
testcase_4()
{
	if [ $MEM_SWAP_FLAG -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo $((PAGESIZE*2048)) > memory.limit_in_bytes
	echo $((PAGESIZE*2048)) > memory.memsw.limit_in_bytes
	test_max_usage_in_bytes "--mmap-anon" $((PAGESIZE*1024)) \
		"memory.memsw.max_usage_in_bytes" $((PAGESIZE*1024)) 1
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

	if [ -e memory.memsw.max_usage_in_bytes ]; then
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
