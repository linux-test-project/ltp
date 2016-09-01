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
# File :        memcg_usage_in_bytes_test.sh
# Description:  Tests memory.max_usage_in_bytes.
# Author:       Peng Haitao <penght@cn.fujitsu.com>
# History:      2012/01/16 - Created.
#

TCID="memcg_usage_in_bytes_test"
TST_TOTAL=2

. memcg_lib.sh

# Test memory.usage_in_bytes
testcase_1()
{
	test_mem_stat "--mmap-anon" $((PAGESIZE*1024)) $((PAGESIZE*1024)) \
		"memory.usage_in_bytes" $((PAGESIZE*1024)) false
}

# Test memory.memsw.usage_in_bytes
testcase_2()
{
	if [ "$MEMSW_USAGE_FLAG" -eq 0 ]; then
		tst_resm TCONF "mem+swap is not enabled"
		return
	fi

	echo $((PAGESIZE*2048)) > memory.limit_in_bytes
	echo $((PAGESIZE*2048)) > memory.memsw.limit_in_bytes
	test_mem_stat "--mmap-anon" $((PAGESIZE*1024)) $((PAGESIZE*1024)) \
		"memory.memsw.usage_in_bytes" $((PAGESIZE*1024)) false
}

run_tests

tst_exit

