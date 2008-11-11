#! /bin/sh 
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##      
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        runpwtests.sh
#
# Description:  
#
# Author:       Nageswara R Sastry <nasastry@in.ibm.com>
#
# History:      26 Aug 2008 - Created this file
# 03 Nov 2008 - Added CPUIDLE sysfs testcase
#

# Exporting Required variables

export TST_TOTAL=1
LTPTMP=${TMP}
LTPROOT=../../bin
export PATH=${PATH}:.
export TCID="Power Management"
export TST_COUNT=0

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0		#Return status

# Checking required kernel version and architecture
check_kv_arch || RC=$?
if [ $RC -eq 1 ] ; then
	tst_resm TCONF "Kernel version or Architecture not supported: Not running testcases"
	exit 0
fi

# Checking sched_mc sysfs interface
#check_config.sh config_sched_mc || RC=$?
if [ -f /sys/devices/system/cpu/sched_mc_power_savings ] ; then
	test_sched_mc.sh || RC=$?
	if [ $RC -eq 1 ] ; then
		tst_resm TFAIL "SCHED_MC sysfs tests failed"
	fi
else
	tst_resm TCONF "Required kernel configuration for SCHED_MC NOT set"
        exit 0
fi

# Checking cpufreq sysfs interface files
#check_config.sh config_cpu_freq || RC=$?
if [ -d /sys/devices/system/cpu/cpu0/cpufreq ] ; then
	check_cpufreq_sysfs_files.sh || RC=$?
	if [ $RC -eq 1 ] ; then
		tst_resm TFAIL "CPUFREQ sysfs tests failed"
	fi

	# Changing governors
	change_govr.sh || RC=$?
	if [ $RC -eq 1 ] ; then
		tst_resm TFAIL "Changing governors failed"
	fi

	# Changing frequencies
	change_freq.sh || RC=$?
	if [ $RC -eq 1 ] ; then
		tst_resm TFAIL "Changing frequncies failed"
	fi

	# Loading and Unloading governor related kernel modules
	pwkm_load_unload.sh || RC=$?
	if [ $RC -eq 1 ] ; then
		tst_resm TFAIL "Loading and Unloading of governor kernel modules got failed"
	fi
else
       tst_resm TCONF "Required kernel configuration for CPU_FREQ NOT set"
       exit 0
fi

# Checking cpuidle sysfs interface files
check_cpuidle_sysfs_files.sh || RC=$?
if [ $RC -eq 1 ] ; then
	tst_resm TFAIL "CPUIDLE sysfs tests failed"
fi
 
