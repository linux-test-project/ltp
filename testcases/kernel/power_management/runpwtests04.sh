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

export TCID="Power_Management04"
export TST_TOTAL=1

. test.sh
. pm_include.sh

check_cpuidle_sysfs_files() {
	RC=0
	if [ -d /sys/devices/system/cpu/cpuidle ] ; then
		for files in current_governor_ro current_driver
		do
			cat /sys/devices/system/cpu/cpuidle/${files} \
				>/dev/null 2>&1
			if [ $? -ne 0 ] ; then
				echo "${0}: FAIL: cat ${files}"
				RC=1
			fi
		done
	fi
	if [ ${RC} -eq 0 ] ; then
		echo "${0}: PASS: Checking cpu idle sysfs files"
	else
		echo "${0}: FAIL: Checking cpu idle sysfs files"
	fi
	return $RC
}

# Checking test environment
check_arch

# Checking cpuidle sysfs interface files
if check_cpuidle_sysfs_files ; then
	tst_resm TPASS "CPUIDLE sysfs tests passed"
else
    tst_resm TFAIL "CPUIDLE sysfs tests failed"
fi

tst_exit
