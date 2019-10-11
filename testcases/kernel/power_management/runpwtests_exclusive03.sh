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

export TCID="Power_Management_exclusive03"
export TST_TOTAL=2

. test.sh
. pm_include.sh

# Checking test environment
check_kervel_arch

if tst_kvcmp -gt "2.6.29"; then
	max_sched_mc=2
	max_sched_smt=2
else
	max_sched_mc=1
	max_sched_smt=1
fi

tst_require_cmds python3

hyper_threaded=$(is_hyper_threaded)
multi_socket=$(is_multi_socket)
multi_core=$(is_multi_core)
if [ $multi_socket -ne 0 -o $multi_core -ne 0 -o \
	$hyper_threaded -ne 0 ]; then
	tst_brkm TCONF "System is not a multi socket & multi core" \
		"& hyper-threaded"
fi

# Verify threads consolidation stops when sched_mc &(/) sched_smt
# is disabled.
# Vary sched_mc from 1/2 to 0 when workload is running and
# ensure that tasks do not consolidate to single package when
# sched_mc is set to 0.
RC=0
for sched_mc in `seq 1  $max_sched_mc`; do
	if pm_cpu_consolidation.py -v -c $sched_mc; then
		echo "Test PASS: CPU consolidation test by varying" \
			"sched_mc $sched_mc to 0"
	else
		RC=1
		echo "Test FAIL: CPU consolidation test by varying" \
			"sched_mc $sched_mc to 0"
	fi
done
if [ $RC -eq 0 ]; then
	tst_resm TPASS "CPU consolidation test by varying sched_mc"
else
	tst_resm TFAIL "CPU consolidation test by varying sched_mc"
fi

# Vary sched_mc & sched_smt from 1 to 0 & 2 to 0 when workload
# is running and ensure that tasks do not consolidate to single
# package when sched_mc is set to 0.
RC=0
for sched_mc in `seq 1  $max_sched_mc`; do
	for sched_smt in `seq 1  $max_sched_smt`; do
		if [ $sched_smt -eq $sched_mc ]; then
			if pm_cpu_consolidation.py -v -c $sched_mc \
				-t $sched_smt; then
				echo "Test PASS: CPU consolidation test by" \
					"varying sched_mc & sched_smt from" \
					"$sched_mc to 0"
			else
				RC=1
				echo "Test FAIL: CPU consolidation test by" \
					"varying sched_mc & sched_smt from" \
					"$sched_mc to 0"
			fi
		fi
	done
done
if [ $RC -eq 0 ]; then
	tst_resm TPASS "CPU consolidation test by varying" \
		"sched_mc & sched_smt"
else
	tst_resm TFAIL "CPU consolidation test by varying" \
		"sched_mc & sched_smt"
fi

tst_exit
