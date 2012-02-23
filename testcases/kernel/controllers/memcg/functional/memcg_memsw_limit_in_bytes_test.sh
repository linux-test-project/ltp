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
# File :        memcg_memsw_limit_in_bytes_test.sh
# Description:  Tests memory.memsw.limit_in_bytes.
# Author:       Peng Haitao <penght@cn.fujitsu.com>
# History:      2012/01/3 - Created.
#

export TCID="memcg_memsw_limit_in_bytes_test"
export TST_TOTAL=12
export TST_COUNT=0

. ./memcg_lib.sh || exit 1

MEM_SWAP_FLAG=0

testcase_1()
{
	test_proc_kill $PAGESIZE "--mmap-lock1" $((PAGESIZE*2)) 1
}

testcase_2()
{
	test_proc_kill $PAGESIZE "--mmap-lock2" $((PAGESIZE*2)) 1
}

testcase_3()
{
	test_proc_kill 0 "--mmap-anon" $PAGESIZE 1
}

testcase_4()
{
	test_proc_kill 0 "--mmap-file" $PAGESIZE 1
}

testcase_5()
{
	test_proc_kill 0 "--shm -k 21" $PAGESIZE 1
}

testcase_6()
{
	test_limit_in_bytes $((PAGESIZE-1)) $PAGESIZE 1
}

testcase_7()
{
	test_limit_in_bytes $((PAGESIZE+1)) $((PAGESIZE*2)) 1
}

testcase_8()
{
	test_limit_in_bytes 1 $PAGESIZE 1
}

testcase_9()
{
	if [ $MEM_SWAP_FLAG -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo 10M > memory.limit_in_bytes
	echo -1 > memory.memsw.limit_in_bytes 2> /dev/null
	ret=$?
	tst_kvercmp 2 6 31
	if [ $? -eq 0 ]; then
		result $(( !($ret != 0) ))  "return value is $ret"
	else
		result $(( !($ret == 0) ))  "return value is $ret"
	fi
}

testcase_10()
{
	if [ $MEM_SWAP_FLAG -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo 10M > memory.limit_in_bytes
	echo 1.0 > memory.memsw.limit_in_bytes 2> /dev/null
	result $(( !($? != 0) )) "return value is $?"
}

testcase_11()
{
	if [ $MEM_SWAP_FLAG -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo 10M > memory.limit_in_bytes
	echo 1xx > memory.memsw.limit_in_bytes 2> /dev/null
	result $(( !($? != 0) )) "return value is $?"
}

testcase_12()
{
	if [ $MEM_SWAP_FLAG -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo 10M > memory.limit_in_bytes
	echo xx > memory.memsw.limit_in_bytes 2> /dev/null
	result $(( !($? != 0) )) "return value is $?"
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
