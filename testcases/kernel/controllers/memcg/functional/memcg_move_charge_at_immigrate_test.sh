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
# File :        memcg_move_charge_at_immigrate_test.sh
# Description:  Tests memory.move_charge_at_immigrate.
# Author:       Peng Haitao <penght@cn.fujitsu.com>
# History:      2012/01/16 - Created.
#

TCID="memcg_move_charge_at_immigrate_test"
TST_TOTAL=4

. memcg_lib.sh

# Test disable moving charges
testcase_1()
{
	test_move_charge "--mmap-anon" $PAGESIZES $PAGESIZES 0 0 0 $PAGESIZES 0
}

# Test move anon
testcase_2()
{
	test_move_charge "--mmap-anon --shm --mmap-file" $PAGESIZES \
		$((PAGESIZES*3)) 1 $PAGESIZES 0 0 $((PAGESIZES*2))
}

# Test move file
testcase_3()
{
	test_move_charge "--mmap-anon --shm --mmap-file" $PAGESIZES \
		$((PAGESIZES*3)) 2 0 $((PAGESIZES*2)) $PAGESIZES 0
}

# Test move anon and file
testcase_4()
{
	test_move_charge "--mmap-anon --shm" $PAGESIZES \
		$((PAGESIZES*2)) 3 $PAGESIZES $PAGESIZES 0 0
}

run_tests

tst_exit
