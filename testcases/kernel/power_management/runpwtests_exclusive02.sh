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

export TCID="Power_Management_exclusive02"
export TST_TOTAL=1

. test.sh
. pm_include.sh

# Checking test environment
check_kervel_arch

if tst_kvcmp -gt "2.6.29"; then
	max_sched_smt=2
else
	max_sched_smt=1
fi

tst_require_cmds python3

hyper_threaded=$(is_hyper_threaded)
multi_socket=$(is_multi_socket)
multi_core=$(is_multi_core)
if [ $hyper_threaded -ne 0 -o $multi_socket -ne 0 \
	-o $multi_core -eq 0 ]; then
	tst_brkm TCONF "System is a multi core but not multi" \
		"socket & hyper-threaded"
fi

#Testcase to validate consolidation at core level
RC=0
for sched_smt in `seq 0 $max_sched_smt`; do
	if [ $sched_smt -eq 2 ]; then
		work_load="kernbench"
	else
		work_load="ebizzy"
	fi
	sched_smt_pass_cnt=0
	stress="thread"
	for repeat_test in `seq 1  10`; do
		if pm_cpu_consolidation.py -t $sched_smt -w $work_load \
			-s $stress; then
			: $(( sched_smt_pass_cnt += 1 ))
		fi
	done
	analyze_core_consolidation_result $sched_smt \
		$sched_smt_pass_cnt; RC=$?
done
if [ $RC -eq 0 ]; then
    tst_resm TPASS "Consolidation test at core level for sched_smt"
else
    tst_resm TFAIL "Consolidation test at core level for sched_smt"
fi

tst_exit
