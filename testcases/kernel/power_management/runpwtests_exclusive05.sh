#! /bin/sh
#
# Copyright (c) International Business Machines  Corp., 2001
# Author: Nageswara R Sastry <nasastry@in.ibm.com>
#
# This program is free software;  you can redistribute it and#or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program;  if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

export TCID="Power_Management_exclusive05"
export TST_TOTAL=2

. test.sh
. pm_include.sh

# Checking test environment
check_arch
check_kernel_version

max_sched_mc=2
max_sched_smt=2

tst_require_cmds python3

hyper_threaded=$(is_hyper_threaded)
multi_socket=$(is_multi_socket)
multi_core=$(is_multi_core)
if [ $multi_socket -ne 0 -o $multi_core -ne 0 -o \
	$hyper_threaded -ne 0 ]; then
	tst_brkm TCONF "System is not a multi socket & multi core" \
		"& hyper-threaded"
fi

# Verify ILB runs in same package as workload.
RC=0
for sched_mc in `seq 1 $max_sched_mc`; do
	if [ $sched_mc -eq 2 ]; then
		work_load="kernbench"
	else
		work_load="ebizzy"
	fi

	pm_ilb_test.py -c $sched_mc -w $work_load
	if [ $? -eq 0 ]; then
		echo "Test PASS: ILB & workload in same package for" \
			"sched_mc=$sched_mc"
	else
		RC=1
		echo "Test FAIL: ILB & workload did not run in same package" \
			"for sched_mc=$sched_mc. Ensure CONFIG_NO_HZ is set"
	fi
done
if [ $RC -eq 0 ]; then
	tst_resm TPASS "ILB & workload test in same package for sched_mc"
else
	tst_resm TFAIL "ILB & workload test in same package for sched_mc"
fi

RC=0
for sched_mc in `seq 1 $max_sched_mc`; do
	if [ $sched_mc -eq 2 ]; then
		work_load="kernbench"
	else
		work_load="ebizzy"
	fi
	for sched_smt in `seq 1 $max_sched_smt`; do
		pm_ilb_test.py -c $sched_mc -t sched_smt -w $work_load
		if [ $? -eq 0 ]; then
			echo "Test PASS: ILB & workload in same package for" \
				"sched_mc=$sched_mc & sched_smt=$sched_smt"
		else
			RC=1
			echo "Test FAIL: ILB & workload did not execute in" \
				"same package for sched_mc=$sched_mc &" \
				"sched_smt=$sched_smt. Ensure CONFIG_NO_HZ is set"
		fi
	done
done
if [ $RC -eq 0 ]; then
	tst_resm TPASS "ILB & workload test in same package for" \
		"sched_mc & sched_smt"
else
	tst_resm TFAIL "ILB & workload test in same package for" \
		"sched_mc & sched_smt"
fi

tst_exit
