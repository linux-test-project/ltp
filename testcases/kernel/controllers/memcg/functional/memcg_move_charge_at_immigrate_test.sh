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
# File :        memcg_move_charge_at_immigrate_test.sh
# Description:  Tests memory.move_charge_at_immigrate.
# Author:       Peng Haitao <penght@cn.fujitsu.com>
# History:      2012/01/16 - Created.
#

export TCID="memcg_move_charge_at_immigrate_test"
export TST_TOTAL=4
export TST_COUNT=0

. ./memcg_lib.sh || exit 1

# Test disable moving charges
testcase_1()
{
	test_move_charge "--mmap-anon" $PAGESIZE  0 0 $PAGESIZE
}

# Test move anon
testcase_2()
{
	test_move_charge "--mmap-anon --shm --mmap-file" $PAGESIZE 1 \
		$PAGESIZE $((PAGESIZE*2))
}

# Test move file
testcase_3()
{
	test_move_charge "--mmap-anon --shm --mmap-file" $PAGESIZE 2 \
		$((PAGESIZE*2)) $PAGESIZE
}

# Test move anon and file
testcase_4()
{
	test_move_charge "--mmap-anon --shm" $PAGESIZE 3 $((PAGESIZE*2)) 0
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
