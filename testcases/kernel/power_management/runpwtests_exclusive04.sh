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

export TCID="Power_Management_exclusive04"
export TST_TOTAL=2

. test.sh
. pm_include.sh

# Checking test environment
check_arch
check_kernel_version

tst_require_cmds python3

hyper_threaded=$(is_hyper_threaded)
multi_socket=$(is_multi_socket)
if [ $hyper_threaded -ne 0 -o $multi_socket -ne 0 ]; then
	tst_brkm TCONF "System is not a multi socket & hyper-threaded"
fi

# Verify threads consolidation stops when sched_smt is
# disabled in HT systems.
# Vary only sched_smt from 1 to 0 when workload is running
# and ensure that tasks do not consolidate to single core
# when sched_smt is set to 0.
if pm_cpu_consolidation.py -v -t 1; then
	tst_resm TPASS "CPU consolidation test by varying sched_smt from 1 to 0"
else
	tst_resm TFAIL "CPU consolidation test by varying sched_smt from 1 to 0"
fi

# Vary only sched_smt from 2 to 0 when workload is running
# and ensure that tasks do not consolidate to single core
# when sched_smt is set to 0.
if pm_cpu_consolidation.py -v -t 2; then
	tst_resm TPASS "CPU consolidation test by varying sched_smt from 2 to 0"
else
	tst_resm TFAIL "CPU consolidation test by varying sched_smt from 2 to 0"
fi

tst_exit
