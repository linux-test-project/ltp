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

TST_ID="cpuacct"
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_TESTFUNC=do_test
TST_POS_ARGS=2
TST_USAGE=usage
TST_NEEDS_ROOT=1

. tst_test.sh

mounted=1
max=$1
nbprocess=$2

usage()
{
	cat << EOF
usage: $0 nsubgroup nprocess

nsubgroup - number of subgroups to create
nprocess  - number of processes to attach to each subgroup

OPTIONS
EOF
}

setup()
{
	if ! grep -q -w cpuacct /proc/cgroups; then
		tst_brk TCONF "cpuacct not supported on this system"
	fi

	mount_point=`grep -w cpuacct /proc/mounts | cut -f 2 | cut -d " " -f2`
	tst_res TINFO "cpuacct: $mount_point"
	if [ "$mount_point" = "" ]; then
		mounted=0
		mount_point=/dev/cgroup
	fi

	testpath=$mount_point/ltp_$TST_ID

	if [ "$mounted" -eq "0" ]; then
		ROD mkdir -p $mount_point
		ROD mount -t cgroup -o cpuacct none $mount_point
	fi

	ROD mkdir $testpath

	# create subgroups
	for i in `seq 1 $max`; do
		ROD mkdir $testpath/subgroup_$i
	done

}

cleanup()
{
	tst_res TINFO "removing created directories"

	if [ -d "$testpath/subgroup_1" ]; then
		rmdir $testpath/subgroup_*
	fi

	rmdir $testpath

	if [ "$mounted" -ne 1 ]; then
		tst_res TINFO "Umounting cpuacct"
		umount $mount_point
		rmdir $mount_point
	fi
}

do_test()
{
	tst_res TINFO "Creating $max subgroups each with $nbprocess processes"

	# create and attach process to subgroups
	for i in `seq 1 $max`; do
		for j in `seq 1 $nbprocess`; do
			cpuacct_task $testpath/subgroup_$i/tasks &
		done
	done

	wait

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
	if [ "$fails" -gt "0" ]; then
		tst_res TFAIL "cpuacct.usage is not equal to 0 for $fails subgroups"
	else
		tst_res TPASS "cpuacct.usage is not equal to 0 for every subgroup"
	fi

	## check that ltp_subgroup/cpuacct.usage == sum ltp_subgroup/subgroup*/cpuacct.usage
	ref=`cat $testpath/cpuacct.usage`
	if [ "$ref" -ne "$acc" ]; then
		tst_res TFAIL "cpuacct.usage $ref not equal to subgroup*/cpuacct.usage $acc"
	else
		tst_res TPASS "cpuacct.usage equal to subgroup*/cpuacct.usage"
	fi
}

tst_run
