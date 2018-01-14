#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
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
# This is a wrapper for locktorture kernel module. The test requires
# that kernel configured with CONFIG_LOCK_TORTURE_TEST. It runs locktorture
# module using particular options and then inspects dmesg output for module's
# test results. For more information, please read Linux Documentation:
# locking/locktorture.txt

TCID="lock_torture"
TST_TOTAL=6
TST_CLEANUP=cleanup
. test.sh

# default options
test_time=60

while getopts :ht: opt; do
	case "$opt" in
	h)
		echo "Usage:"
		echo "h        help"
		echo "t x      time in seconds for each test-case"
		exit 0
	;;
	t) test_time=$OPTARG ;;
	*)
		tst_brkm TBROK "unknown option: $opt"
	;;
	esac
done

cleanup()
{
	tst_resm TINFO "cleanup"
	rmmod locktorture > /dev/null 2>&1
}

if tst_kvcmp -lt "3.18"; then
	tst_brkm TCONF "test must be run with kernel 3.18 or newer"
fi

tst_require_root

# check if module is present
modprobe locktorture > /dev/null 2>&1 || \
	tst_brkm TCONF "Test requires locktorture module"
rmmod locktorture > /dev/null 2>&1

trap cleanup INT

lock_type="spin_lock spin_lock_irq rw_lock rw_lock_irq mutex_lock rwsem_lock"

est_time=$(echo "scale=2; $test_time * $TST_TOTAL / 60 " | bc)
tst_resm TINFO "estimate time $est_time min"

for type in $lock_type; do

	tst_resm TINFO "$type: running $test_time sec..."

	modprobe locktorture torture_type=$type \
	         > /dev/null 2>&1 || tst_brkm TBROK "failed to load module"

	sleep $test_time

	rmmod locktorture > /dev/null 2>&1 || \
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
