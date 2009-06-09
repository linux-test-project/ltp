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
export PATH=${PATH}:.
export TCID="Power Management"
export TST_COUNT=0

#List of reusable functions defined in pm_include.sh
. pm_include.sh

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0		#Return status

# Checking required kernel version and architecture
check_kv_arch; RC=$?
if [ $RC -eq 1 ] ; then
	tst_resm TCONF "Kernel version or Architecture not supported:\
Not running testcases"
	exit 0
fi

# Checking sched_mc sysfs interface
#check_config.sh config_sched_mc || RC=$?
if [ -f /sys/devices/system/cpu/sched_mc_power_savings ] ; then
	if test_sched_mc.sh ; then
		tst_resm TPASS "SCHED_MC sysfs tests passed"
	else
		RC=$?
		tst_resm TFAIL "SCHED_MC sysfs tests failed"
	fi
	# Test CPU consolidation for corresponding sched_mc
	which python > /dev/null
	if [ $? -ne 0 ] ; then
		tst_resm TCONF "Python is not installed, CPU Consoldation\
		 test not run"
	else
		# Test CPU consolidation on hyper-threaded system
		hyper_threaded=$(is_hyper_threaded)
		if [ $hyper_threaded -eq 1 ]; then
			for sched_mc in `seq 0 2`; do
				for sched_smt in `seq 0 1`; do
					if [ $sched_smt -eq 0 -a $sched_mc -eq 0 ]; then 
						continue
					fi
					if cpu_consolidation.py -c $sched_mc -t $sched_smt ; then
						tst_resm TPASS "cpu consolidation sched_mc=$sched_mc,\
 sched_smt=$sched_smt"
					else
						RC=$?
						tst_resm TFAIL "cpu consolidation \
 sched_mc=$sched_mc, sched_smt=$sched_smt"
					fi
				done
			done
		else
			# Test CPU consolidation for sched_mc=1 & 2
			for sched_mc in `seq 1 2`; do
				if cpu_consolidation.py -c $sched_mc ; then
					tst_resm TPASS "cpu consolidation test for \
sched_mc_power set to $sched_mc"
				else
					RC=$?
					tst_resm TFAIL "cpu consolidation test\
 sched_mc_power set to $sched_mc"
				fi
			done
		fi

		# Testcase to validate sched_domain tree
		if [ $hyper_threaded -eq 1 ]; then
			for sched_mc in `seq 0 2`; do
                for sched_smt in `seq 0 1`; do
					if [ $sched_smt -eq 0 -a $sched_mc -eq 0 ]; then
                        continue
                    fi

					if sched_domain.py -c $sched_mc -t $sched_smt; then
						tst_resm TPASS "sched domain test sched_mc=$sched_mc,\
sched_smt=$sched_smt "
					else
						RC=$?
						tst_resm TFAIL "sched domain test sched_mc=$sched_mc,\
sched_smt=$sched_smt "
					fi
				done
			done
		else
			# Validate CPU level sched domain topology validation
			for sched_mc in `seq 1 2`; do
				if sched_domain.py -c $sched_mc ; then
					tst_resm TPASS "sched domain test for sched_mc=$sched_mc "
				else
					RC=$?
					tst_resm TFAIL "sched domain test for sched_mc=$sched_mc "
				fi
			done
			
		fi
	fi
else
	tst_resm TCONF "Required kernel configuration for SCHED_MC NOT set"
fi
# Checking cpufreq sysfs interface files
#check_config.sh config_cpu_freq || RC=$?
if [ -d /sys/devices/system/cpu/cpu0/cpufreq ] ; then
	if check_cpufreq_sysfs_files.sh ; then
		tst_resm TPASS "CPUFREQ sysfs tests "
	else
		RC=$?
		tst_resm TFAIL "CPUFREQ sysfs tests "
	fi

	# Changing governors
	if change_govr.sh ; then
		tst_resm TPASS "Changing governors "
	else
		RC=$?
		tst_resm TFAIL "Changing governors "
	fi

	# Changing frequencies
	if change_freq.sh ; then
		tst_resm TPASS "Changing frequncies "
	else
		RC=$?
		tst_resm TFAIL "Changing frequncies "
	fi

	# Loading and Unloading governor related kernel modules
	if pwkm_load_unload.sh ; then
		tst_resm TPASS "Loading and Unloading of governor kernel \
		modules got failed"
	else
		RC=$?
		tst_resm TFAIL "Loading and Unloading of governor kernel \
		modules got failed"
	fi
else
	tst_resm TCONF "Required kernel configuration for CPU_FREQ NOT set"
fi

# Checking cpuidle sysfs interface files
if check_cpuidle_sysfs_files.sh ; then
	tst_resm TPASS "CPUIDLE sysfs tests failed"
else
	RC=$?
	tst_resm TFAIL "CPUIDLE sysfs tests failed"
fi

# Test sched_smt_power_savings interface on HT machines
if [ -f /sys/devices/system/cpu/sched_smt_power_savings ] ; then
	if test_sched_smt.sh ; then
		tst_resm TPASS "SCHED_MC sysfs tests failed"
	else
		RC=$?
		tst_resm TFAIL "SCHED_MC sysfs tests failed"
	fi
else
	hyper_threaded=$(is_hyper_threaded)
	if [ $hyper_threaded -eq 1 ]; then
		tst_resm TFAIL "Required kernel configuration for SCHED_SMT NOT set"
		RC=1
	else
		tst_resm TCONF "Required Hyper Threading support in the\
system under test"
	fi
fi

exit $RC