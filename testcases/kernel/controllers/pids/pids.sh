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

caseno=$1
max=$2
export TCID="pids_$1_$2"
export TST_TOTAL=1
mounted=1

. test.sh

cleanup()
{
	killall -9 pids_task2 >/dev/null 2>&1

	tst_resm TINFO "removing created directories"
	rmdir $testpath
	if [ "$mounted" -ne "1" ]; then
		tst_resm TINFO "Umounting pids"
		umount $mount_point
		rmdir $mount_point
	fi
}

setup()
{
	tst_require_root

	exist=`grep -w pids /proc/cgroups | cut -f1`;
	if [ "$exist" = "" ]; then
		tst_brkm TCONF NULL "pids not supported"
	fi

	mount_point=`grep -w pids /proc/mounts | cut -f 2 | cut -d " " -f2`

	if [ "$mount_point" = "" ]; then
		mounted=0
		mount_point=/dev/cgroup
	fi

	TST_CLEANUP=cleanup

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
		tst_resm TBROK "failed to attach process"
	fi
}

case1()
{
	start_pids_tasks2 $max

	# should return 0 because there is no limit
	pids_task1 "$testpath/tasks"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_resm TFAIL "fork failed unexpectedly"
	elif [ "$ret" -eq "0" ]; then
		tst_resm TPASS "fork didn't fail"
	else
		tst_resm TBROK "pids_task1 failed"
	fi
}

case2()
{
	tmp=$((max - 1))
	tst_resm TINFO "limit the number of pid to $max"
	ROD echo $max \> $testpath/pids.max

	start_pids_tasks2 $tmp

	# should return 2 because the limit of pids is reached
	pids_task1 "$testpath/tasks"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_resm TPASS "fork failed as expected"
	elif [ "$ret" -eq "0" ]; then
		tst_resm TFAIL "fork didn't fail despite the limit"
	else
		tst_resm TBROK "pids_task1 failed"
	fi
}

case3()
{
	lim=$((max + 2))
	tst_resm TINFO "limit the number of avalaible pid to $lim"
	ROD echo $lim \> $testpath/pids.max

	start_pids_tasks2 $max

	pids_task1 "$testpath/tasks"
	ret=$?

	if [ "$ret" -eq "2" ]; then
		tst_resm TFAIL "fork failed unexpectedly"
	elif [ "$ret" -eq "0" ]; then
		tst_resm TPASS "fork worked as expected"
	else
		tst_resm TBROK "pids_task1 failed"
	fi
}

case4()
{
	tst_resm TINFO "limit the number of avalaible pid to 0"
	ROD echo 0 \> $testpath/pids.max

	start_pids_tasks2 $max

	tst_resm TPASS "all process were attached"
}

case5()
{
	tst_resm TINFO "try to limit the number of avalaible pid to -1"
	echo -1 > $testpath/pids.max

	if [ "$?" -eq "0" ]; then
		tst_resm TFAIL "managed to set the limit to -1"
	else
		tst_resm TPASS "didn't manage to set the limit to -1"
	fi
}

setup

if [ "$#" -ne "2" ]; then
	tst_resm TFAIL "invalid parameters, usage: ./pids01 caseno max"
else
	case$caseno
fi

tst_exit
