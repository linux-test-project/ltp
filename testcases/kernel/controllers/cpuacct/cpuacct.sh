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
# ./cpuacct.sh nbsubgroup nbprocess
#
# 1) nbsubgroup : number of subgroup to create
# 2) nbprocess : number of process to attach to each subgroup
#
# Description
#
# 1) Find if cpuacct is mounted, if not mounted, cpuacct will be mounted
# 2) Check that sum ltp_test/subgroup*/cpuacct.usage = ltp_test/cpuacct.usage
#

mounted=1
max=$1
nbprocess=$2

export TCID="cpuacct_$1_$2"
export TST_TOTAL=2

. test.sh

setup()
{
	tst_require_root

	grep -q -w cpuacct /proc/cgroups
	if [ $? -ne 0 ]; then
		tst_brkm TCONF "cpuacct not supported on this system"
	fi

	mount_point=`grep -w cpuacct /proc/mounts | cut -f 2 | cut -d " " -f2`
	tst_resm TINFO "cpuacct: $mount_point"
	if [ "$mount_point" = "" ]; then
		mounted=0
		mount_point=/dev/cgroup
	fi

	TST_CLEANUP=cleanup

	testpath=$mount_point/ltp_$TCID

	if [ "$mounted" -eq "0" ]; then
		ROD mkdir -p $mount_point
		ROD mount -t cgroup -o cpuacct none $mount_point
	fi
	ROD mkdir $testpath
}

cleanup()
{
	tst_resm TINFO "removing created directories"
	rmdir $testpath/subgroup_*
	rmdir $testpath
	if [ "$mounted" -ne 1 ]; then
		tst_resm TINFO "Umounting cpuacct"
		umount $mount_point
		rmdir $mount_point
	fi
}

setup;

# create subgroups
for i in `seq 1 $max`; do
	ROD mkdir -p $testpath/subgroup_$i
done

# create and attach process to subgroups
for i in `seq 1 $max`; do
	for j in `seq 1 $nbprocess`; do
		cpuacct_task $testpath/subgroup_$i/tasks &
	done
done

for job in `jobs -p`; do
	wait $job
done

acc=0
fails=0
for i in `seq 1 $max`; do
	tmp=`cat $testpath/subgroup_$i/cpuacct.usage`
	if [ "$tmp" -eq "0" ]; then
		fails=$((fails + 1))
	fi
	acc=$((acc + tmp))
done

## check that cpuacct.usage != 0 for every subgroup
if [ "$fails" -eq "1" ]; then
	tst_resm TFAIL "cpuacct.usage is not equal to 0 for $fails subgroups"
else
	tst_resm TPASS "cpuacct.usage is not equal to 0 for every subgroup"
fi

## check that ltp_subgroup/cpuacct.usage == sum ltp_subgroup/subgroup*/cpuacct.usage
ref=`cat $testpath/cpuacct.usage`
if [ "$ref" != "$acc" ]; then
	tst_resm TFAIL "ltp_test/cpuacct.usage not equal to ltp_test/subgroup*/cpuacct.usage"
else
	tst_resm TPASS "ltp_test/cpuacct.usage equal to ltp_test/subgroup*/cpuacct.usage"
fi

tst_exit
