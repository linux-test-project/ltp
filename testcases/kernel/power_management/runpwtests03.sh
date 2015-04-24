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

export TCID="Power_Management03"
export TST_TOTAL=4

. test.sh
. pm_include.sh

# Checking test environment
check_kervel_arch

# Checking cpufreq sysfs interface files
if [ ! -d /sys/devices/system/cpu/cpu0/cpufreq ] ; then
	tst_brkm TCONF "Required kernel configuration for CPU_FREQ NOT set"
fi

if check_cpufreq_sysfs_files.sh ; then
	tst_resm TPASS "CPUFREQ sysfs tests"
else
	tst_resm TFAIL "CPUFREQ sysfs tests"
fi

# Changing governors
if change_govr.sh ; then
	tst_resm TPASS "Changing governors"
else
	tst_resm TFAIL "Changing governors"
fi

# Changing frequencies
if change_freq.sh ; then
	tst_resm TPASS "Changing frequncies"
else
    tst_resm TFAIL "Changing frequncies"
fi

# Loading and Unloading governor related kernel modules
if pwkm_load_unload.sh ; then
	tst_resm TPASS "Loading and Unloading of governor kernel" \
		"modules"
else
	tst_resm TFAIL "Loading and Unloading of governor kernel" \
		"modules got failed"
fi

tst_exit
