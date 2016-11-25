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
# File :        memcg_memsw_limit_in_bytes_test.sh
# Description:  Tests memory.memsw.limit_in_bytes.
# Author:       Peng Haitao <penght@cn.fujitsu.com>
# History:      2012/01/3 - Created.
#

TCID="memcg_memsw_limit_in_bytes_test"
TST_TOTAL=12

. memcg_lib.sh

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
	test_limit_in_bytes $((PAGESIZE-1)) 1
}

testcase_7()
{
	test_limit_in_bytes $((PAGESIZE+1)) 1
}

testcase_8()
{
	test_limit_in_bytes 1 1
}

testcase_9()
{
	if [ "$MEMSW_LIMIT_FLAG" -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo 10M > memory.limit_in_bytes

	if tst_kvcmp -lt "2.6.31"; then
		EXPECT_FAIL echo -1 \> memory.memsw.limit_in_bytes
	else
		EXPECT_PASS echo -1 \> memory.memsw.limit_in_bytes
	fi
}

testcase_10()
{
	if [ "$MEMSW_LIMIT_FLAG" -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo 10M > memory.limit_in_bytes
	EXPECT_FAIL echo 1.0 \> memory.memsw.limit_in_bytes
}

testcase_11()
{
	if [ "$MEMSW_LIMIT_FLAG" -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo 10M > memory.limit_in_bytes
	EXPECT_FAIL echo 1xx \> memory.memsw.limit_in_bytes
}

testcase_12()
{
	if [ "$MEMSW_LIMIT_FLAG" -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo 10M > memory.limit_in_bytes
	EXPECT_FAIL echo xx \> memory.memsw.limit_in_bytes
}

run_tests

tst_exit
