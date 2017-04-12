#! /bin/sh

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
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
## Author: Li Zefan <lizf@cn.fujitsu.com>                                     ##
## Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>                     ##
## Added memcg enable/disable functinality: Rishikesh K Rajak                 ##
##                                              <risrajak@linux.vnet.ibm.com  ##
##                                                                            ##
################################################################################

cd $LTPROOT/testcases/bin
export TCID="memcg_stress_test"
export TST_TOTAL=2
export TST_COUNT=0

if [ "x$(grep -w memory /proc/cgroups | cut -f4)" != "x1" ]; then
        echo "WARNING:";
        echo "Either Kernel does not support for memory resource controller or feature not enabled";
        echo "Skipping all memcgroup testcases....";
        exit 0
fi

RUN_TIME=$(( 15 * 60 ))

cleanup()
{
	if [ -e /dev/memcg ]; then
		umount /dev/memcg 2>/dev/null
		rmdir /dev/memcg 2>/dev/null
	fi
}


do_mount()
{
	cleanup;

	mkdir /dev/memcg 2> /dev/null
	mount -t cgroup -omemory memcg /dev/memcg
}


# Run the stress test
#
# $1 - Number of cgroups
# $2 - Allocated how much memory in one process? in MB
# $3 - The interval to touch memory in a process
# $4 - How long does this test run ? in second
run_stress()
{
	do_mount;

	for i in $(seq 0 $(($1-1)))
	do
		mkdir /dev/memcg/$i 2> /dev/null
		./memcg_process_stress $2 $3 &
		eval pid$i=$!

		eval echo \$pid$i > /dev/memcg/$i/tasks
	done

	for i in $(seq 0 $(($1-1)))
	do
		eval /bin/kill -s SIGUSR1 \$pid$i 2> /dev/null
	done

	sleep $4

	for i in $(seq 0 $(($1-1)))
	do
		eval /bin/kill -s SIGKILL \$pid$i 2> /dev/null
		eval wait \$pid$i

		rmdir /dev/memcg/$i 2> /dev/null
	done

	cleanup;
}

testcase_1()
{
	run_stress 150 $(( ($mem-150) / 150 )) 5 $RUN_TIME

	tst_resm TPASS "stress test 1 passed"
}

testcase_2()
{
	run_stress 1 $mem 5 $RUN_TIME

	tst_resm TPASS "stress test 2 passed"
}

echo 3 > /proc/sys/vm/drop_caches
sleep 2
mem_free=`cat /proc/meminfo | grep MemFree | awk '{ print $2 }'`
swap_free=`cat /proc/meminfo | grep SwapFree | awk '{ print $2 }'`

mem=$(( $mem_free + $swap_free / 2 ))
mem=$(( mem / 1024 ))

date
export TST_COUNT=$(( $TST_COUNT + 1 ))
testcase_1
export TST_COUNT=$(( $TST_COUNT + 1 ))
testcase_2
date

exit 0
