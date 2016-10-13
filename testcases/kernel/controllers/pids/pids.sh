#!/bin/sh

################################################################################
##                                                                            ##
## Copyright (c) 2015 SUSE                                                    ##
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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301   ##
## USA                                                                        ##
##                                                                            ##
## Author: Cedric Hnyda <chnyda@suse.com>                                     ##
##                                                                            ##
################################################################################

# Usage
# ./pids.sh caseno max
#
TST_ID="pids"
TST_CLEANUP=cleanup
TST_SETUP=setup
TST_TESTFUNC=do_test
TST_POS_ARGS=2
TST_USAGE=usage
TST_NEEDS_ROOT=1

. tst_test.sh

caseno=$1
max=$2
mounted=1

usage()
{
	cat << EOF
usage: $0 caseno max_processes

caseno        - testcase number from interval 1-5
max_processes - maximal number of processes to attach
                (applicable to testcase 1-4)
OPTIONS
EOF
}

cleanup()
{
	killall -9 pids_task2 >/dev/null 2>&1

	tst_res TINFO "removing created directories"
	rmdir $testpath
	if [ "$mounted" -ne "1" ]; then
		tst_res TINFO "Umounting pids"
		umount $mount_point
		rmdir $mount_point
	fi
}

setup()
{
	exist=`grep -w pids /proc/cgroups | cut -f1`;
	if [ "$exist" = "" ]; then
		tst_brk TCONF NULL "pids not supported"
	fi

	mount_point=`grep -w pids /proc/mounts | cut -f 2 | cut -d " " -f2`

	if [ "$mount_point" = "" ]; then
		mounted=0
		mount_point=/dev/cgroup
	fi

	testpath=$mount_point/ltp_$TCID

	if [ "$mounted" -eq "0" ]; then
		ROD mkdir -p $mount_point
		ROD mount -t cgroup -o pids none $mount_point
	fi
	ROD mkdir -p $testpath
}

start_pids_tasks2()
{
	nb=$1
	for i in `seq 1 $nb`; do
		pids_task2 &
		echo $! > $testpath/tasks
	done

	if [ $(cat "$testpath/tasks" | wc -l) -ne $nb ]; then
		tst_brk TBROK "failed to attach process"
	fi
}

stop_pids_tasks()
{
	for i in `cat $testpath/tasks`; do
		ROD kill -9 $i
		wait $i
	done
}

case1()
{
	start_pids_tasks2 $max

	# should return 0 because there is no limit
	pids_task1 "$testpath/tasks"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_res TFAIL "fork failed unexpectedly"
	elif [ "$ret" -eq "0" ]; then
		tst_res TPASS "fork didn't fail"
	else
		tst_res TBROK "pids_task1 failed"
	fi

	stop_pids_tasks
}

case2()
{
	tmp=$((max - 1))
	tst_res TINFO "limit the number of pid to $max"
	ROD echo $max \> $testpath/pids.max

	start_pids_tasks2 $tmp

	# should return 2 because the limit of pids is reached
	pids_task1 "$testpath/tasks"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_res TPASS "fork failed as expected"
	elif [ "$ret" -eq "0" ]; then
		tst_res TFAIL "fork didn't fail despite the limit"
	else
		tst_res TBROK "pids_task1 failed"
	fi

	stop_pids_tasks
}

case3()
{
	lim=$((max + 2))
	tst_res TINFO "limit the number of avalaible pid to $lim"
	ROD echo $lim \> $testpath/pids.max

	start_pids_tasks2 $max

	pids_task1 "$testpath/tasks"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_res TFAIL "fork failed unexpectedly"
	elif [ "$ret" -eq "0" ]; then
		tst_res TPASS "fork worked as expected"
	else
		tst_res TBROK "pids_task1 failed"
	fi

	stop_pids_tasks
}

case4()
{
	tst_res TINFO "limit the number of avalaible pid to 0"
	ROD echo 0 \> $testpath/pids.max

	start_pids_tasks2 $max

	tst_res TPASS "all process were attached"

	stop_pids_tasks
}

case5()
{
	tst_res TINFO "try to limit the number of avalaible pid to -1"
	echo -1 > $testpath/pids.max

	if [ "$?" -eq "0" ]; then
		tst_res TFAIL "managed to set the limit to -1"
	else
		tst_res TPASS "didn't manage to set the limit to -1"
	fi
}

do_test()
{
	tst_res TINFO "Running testcase $caseno with $max processes"
	case$caseno
}

tst_run
