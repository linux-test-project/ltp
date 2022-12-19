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

export TCID="Power_Management05"
export TST_TOTAL=2

. test.sh
. pm_include.sh

# Checking test environment
check_arch

max_sched_mc=2
max_sched_smt=2

tst_require_cmds python3

if ! grep sched_debug -qw /proc/cmdline ; then
	tst_brkm TCONF "Kernel cmdline parameter 'sched_debug' needed," \
		"CPU Consolidation test cannot run"
fi

hyper_threaded=$(is_hyper_threaded)
if [ ! -f /sys/devices/system/cpu/sched_mc_power_savings \
	-o $hyper_threaded -ne 0 ] ; then
	tst_brkm TCONF "Required kernel configuration for SCHED_MC" \
		"NOT set, or sched_mc_power_savings interface in system" \
		"which is not hyper-threaded"
fi

# sched_domain test
echo "max sched mc $max_sched_mc"
RC=0
for sched_mc in `seq 0 $max_sched_mc`; do
	pm_sched_domain.py -c $sched_mc; ret=$?
	analyze_sched_domain_result $sched_mc $ret; RC=$?
done
if [ $RC -eq 0 ]; then
	tst_resm TPASS "Sched_domain test for sched_mc"
else
	tst_resm TFAIL "Sched_domain test for sched_mc"
fi

# Testcase to validate sched_domain tree
RC=0
for sched_mc in `seq 0 $max_sched_mc`; do
	pm_get_sched_values sched_smt; max_sched_smt=$?
	for sched_smt in `seq 0 $max_sched_smt`; do
		pm_sched_domain.py -c $sched_mc -t $sched_smt; ret=$?
		analyze_sched_domain_result $sched_mc $ret $sched_smt; RC=$?
	done
done
if [ $RC -eq 0 ]; then
	tst_resm TPASS "Sched_domain test for sched_mc & sched_smt"
else
	tst_resm TFAIL "Sched_domain test for sched_mc & sched_smt"
fi

tst_exit
