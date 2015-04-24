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

export TCID="Power_Management01"
export TST_TOTAL=1

. test.sh
. pm_include.sh

# Checking test environment
check_kervel_arch

# Checking sched_mc sysfs interface
multi_socket=$(is_multi_socket)
multi_core=$(is_multi_core)
if [ ! -f /sys/devices/system/cpu/sched_mc_power_savings ] ; then
	tst_brkm TCONF "Required kernel configuration for SCHED_MC" \
		"NOT set"
else
	if [ $multi_socket -ne 0 -a $multi_core -ne 0 ] ; then
		tst_brkm TCONF "sched_mc_power_savings interface in system" \
			"which is not a multi socket &(/) multi core"
	fi
fi

if test_sched_mc.sh ; then
	tst_resm TPASS "SCHED_MC sysfs tests"
else
	tst_resm TFAIL "SCHED_MC sysfs tests"
fi

tst_exit
