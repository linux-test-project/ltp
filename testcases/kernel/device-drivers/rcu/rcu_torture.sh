#!/bin/sh
# Copyright (c) 2014-2015 Oracle and/or its affiliates. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# One of the possible ways to test RCU is to use rcutorture kernel module.
# The test requires that kernel configured with CONFIG_RCU_TORTURE_TEST.
# It runs rcutorture module using particular options and then inspects
# dmesg output for module's test results.
# For more information, please read Linux Documentation: RCU/torture.txt

TCID="rcu_torture"
TST_TOTAL=14
TST_CLEANUP=cleanup

. test.sh

# default options
test_time=60
num_writers=5

while getopts :ht:w: opt; do
	case "$opt" in
	h)
		echo "Usage:"
		echo "h        help"
		echo "t x      time in seconds for each test-case"
		echo "w x      number of writers"
		exit 0
	;;
	t) test_time=$OPTARG ;;
	w) num_writers=$OPTARG ;;
	*)
		tst_brkm TBROK "unknown option: $opt"
	;;
	esac
done

cleanup()
{
	tst_resm TINFO "cleanup"
	rmmod rcutorture > /dev/null 2>&1
}

tst_require_root

# check if module is present
modprobe rcutorture > /dev/null 2>&1 || \
	tst_brkm TCONF "Test requires rcutorture module"
rmmod rcutorture > /dev/null 2>&1

trap cleanup INT

rcu_type="rcu rcu_bh srcu sched"

if tst_kvcmp -lt "3.12"; then
	rcu_type="$rcu_type rcu_sync rcu_expedited rcu_bh_sync rcu_bh_expedited \
	          srcu_sync srcu_expedited sched_sync sched_expedited"

	if tst_kvcmp -lt "3.11"; then
		rcu_type="$rcu_type srcu_raw srcu_raw_sync"
	fi
fi

TST_TOTAL=$(echo "$rcu_type" | wc -w)

est_time=`echo "scale=2; $test_time * $TST_TOTAL / 60 " | bc`
tst_resm TINFO "estimate time $est_time min"

for type in $rcu_type; do

	tst_resm TINFO "$type: running $test_time sec..."

	modprobe rcutorture nfakewriters=$num_writers \
	         stat_interval=60 test_no_idle_hz=1 shuffle_interval=3 \
	         stutter=5 irqreader=1 fqs_duration=0 fqs_holdoff=0 \
	         fqs_stutter=3 test_boost=1 test_boost_interval=7 \
	         test_boost_duration=4 shutdown_secs=0 \
	         stall_cpu=0 stall_cpu_holdoff=10 n_barrier_cbs=0 \
	         onoff_interval=0 onoff_holdoff=0 torture_type=$type \
	         > /dev/null 2>&1 || tst_brkm TBROK "failed to load module"

	sleep $test_time

	rmmod rcutorture > /dev/null 2>&1 || \
		tst_brkm TBROK "failed to unload module"

	# check module status in dmesg
	result_str=`dmesg | sed -nE '$s/.*End of test: ([A-Z]+):.*/\1/p'`
	if [ "$result_str" = "SUCCESS" ]; then
		tst_resm TPASS "$type: completed"
	else
		tst_resm TFAIL "$type: $result_str, see dmesg"
	fi
done

tst_exit
