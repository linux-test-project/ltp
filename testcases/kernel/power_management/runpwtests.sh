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
echo Are we really executing this ?
export TST_TOTAL=1
#LTPTMP=${TMP}
export PATH=${PATH}:.
export TCID="Power_Management"
export TST_COUNT=0
export contacts="mpnayak@linux.vnet.ibm.com"
export analysis="/proctstat"

YES=0
NO=1
#List of reusable functions defined in pm_include.sh
. ./pm_include.sh

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0		#Return status

# Checking required kernel version and architecture
check_kv_arch
RC=$?
if [ $RC -eq 1 ] ; then
	tst_resm TCONF "Kernel version or Architecture not supported: Not running testcases"
	exit 0
fi

is_hyper_threaded; hyper_threaded=$?
is_multi_socket; multi_socket=$?
is_multi_core; multi_core=$?
is_dual_core; dual_core=$?

#Checking sched_mc sysfs interface
#check_config.sh config_sched_mc || RC=$?
TST_COUNT=1
if [ $multi_socket -eq $YES -a $multi_core -eq $YES ] ; then
	if [ -f /sys/devices/system/cpu/sched_mc_power_savings ] ; then
		if test_sched_mc.sh ; then
			tst_resm TPASS "SCHED_MC sysfs tests"
		else
			RC=$?
			tst_resm TFAIL "SCHED_MC sysfs tests"
		fi
	else
    	tst_resm TCONF "Required kernel configuration for SCHED_MC NOT set"
	fi
else
	if [ -f /sys/devices/system/cpu/sched_mc_power_savings ] ; then
		tst_resm TFAIL "sched_mc_power_savings interface in system which is not a multi socket &(/) multi core"
	else
		tst_resm TCONF "Not a suitable architecture for SCHED_MC test"
	fi
fi

# Test sched_smt_power_savings interface on HT machines
: $(( TST_COUNT += 1 ))
if [ $hyper_threaded -eq $YES ]; then
	if [ -f /sys/devices/system/cpu/sched_smt_power_savings ] ; then
    	if test_sched_smt.sh; then
			tst_resm TPASS "SCHED_SMT sysfs test"
		else
			RC=$?
        	tst_resm TFAIL "SCHED_SMT sysfs test"
    	fi
	else
		RC=$?
		tst_resm TFAIL "Required kernel configuration for SCHED_SMT NOT set"
	fi
else
	if [ -f /sys/devices/system/cpu/sched_smt_power_savings ] ; then
		RC=$?
        tst_resm TFAIL "sched_smt_power_saving interface in system not hyper-threaded"
    else
        tst_resm TCONF "Required Hyper Threading support for SCHED_SMT test"
    fi
fi

# Checking cpufreq sysfs interface files
#check_config.sh config_cpu_freq || RC=$?
: $(( TST_COUNT += 1 ))
if [ -d /sys/devices/system/cpu/cpu0/cpufreq ] ; then
    if check_cpufreq_sysfs_files.sh; then
		tst_resm TPASS "CPUFREQ sysfs tests"
	else
		RC=$?
		tst_resm TFAIL "CPUFREQ sysfs tests "
	fi

    # Changing governors
	: $(( TST_COUNT += 1 ))
	if change_govr.sh; then
		tst_resm TPASS "Changing governors "
	else
		RC=$?
		tst_resm TFAIL "Changing governors "
	fi

    # Changing frequencies
	: $(( TST_COUNT += 1 ))
    if change_freq.sh ; then
		tst_resm TPASS "Changing frequncies "
	else
		RC=$?
        tst_resm TFAIL "Changing frequncies "
    fi

    # Loading and Unloading governor related kernel modules
	: $(( TST_COUNT += 1 ))
    if pwkm_load_unload.sh ; then
		tst_resm TPASS "Loading and Unloading of governor kernel \
modules"
	else
		RC=$?
        tst_resm TFAIL "Loading and Unloading of governor kernel \
        modules got failed"
    fi
else
    tst_resm TCONF "Required kernel configuration for CPU_FREQ NOT set"
fi

# Checking cpuidle sysfs interface files
: $(( TST_COUNT+=1))
if check_cpuidle_sysfs_files.sh ; then
	tst_resm TPASS "CPUIDLE sysfs tests passed"
else
	RC=$?
    tst_resm TFAIL "CPUIDLE sysfs tests failed"
fi

# sched_domain test
which python > /dev/null
if [ $? -ne 0 ] ; then
	tst_resm TCONF "Python is not installed, CPU Consoldation\
test cannot run"
else
	get_sched_values sched_mc; max_sched_mc=$?
	for sched_mc in `seq 0 $max_sched_mc`; do
		: $(( TST_COUNT+=1))
		sched_domain.py -c $sched_mc; RC=$?
		analyze_sched_domain_result $sched_mc $RC 
		if [ $hyper_threaded -eq $YES ]; then
			get_sched_values sched_smt; max_sched_smt=$?
			for sched_smt in `seq 0 $max_sched_smt`; do
				# Testcase to validate sched_domain tree
				: $(( TST_COUNT+=1))
				sched_domain.py -c $sched_mc -t $sched_smt; RC=$?
				analyze_sched_domain_result $sched_mc $RC $sched_smt ;
			done
		fi
	done
fi

: $(( TST_COUNT+=1))
if [ -f /proc/sys/kernel/timer_migration ]; then
	if test_timer_migration.sh; then
        tst_resm TPASS "Timer Migration interface test"
    else
        RC=$?
        tst_resm TFAIL "Timer migration interface test"
    fi
else
	check_kv_arch "timer_migration"; supp=$? 
	if [ $supp -eq $YES ]; then
		RC=$?
		tst_resm TFAIL "Timer migration interface missing"
	else
		tst_resm TCONF "Kernel version does not support Timer migration"
	fi
fi

: $(( TST_COUNT+=1))	
if check_cpufreq_sysfs_files.sh; then
        tst_resm TPASS "CPUFREQ sysfs tests"
    else
        RC=$?
        tst_resm TFAIL "CPUFREQ sysfs tests "
    fi

if [ $# -gt 0 -a "$1" = "-exclusive" ]; then 
	# Test CPU consolidation 
	if [ $multi_socket -eq $YES -a $multi_core -eq $YES ]; then
		work_loads_list="ebizzy kernbench"
		for sched_mc in `seq 0 $max_sched_mc`; do
			for work_load in ${work_loads_list}
           	do
				: $(( TST_COUNT += 1 ))
				sched_mc_pass_cnt=0
				for repeat_test in `seq 1  10`; do
					#Testcase to validate CPU consolidation for sched_mc
					if cpu_consolidation.py -c $sched_mc -w $work_load ; then
						: $(( sched_mc_pass_cnt += 1 ))
					fi
				done
				analyze_package_consolidation_result $sched_mc $work_load $sched_mc_pass_cnt	
			done	
			if [ $hyper_threaded -eq $YES ]; then
				for sched_smt in `seq 0 $max_sched_smt`; do
					for work_load in ${work_loads_list}; do
                   				: $(( TST_COUNT += 1 ))
						sched_mc_smt_pass_cnt=0
						for repeat_test in `seq 1  10`; do
							# Testcase to validate CPU consolidation for
							# for sched_mc & sched_smt with stress=50%
							if cpu_consolidation.py -c $sched_mc -t $sched_smt -w $work_load; then
								: $(( sched_mc_smt_pass_cnt += 1 ))
							fi
							echo "sched_mc_smt_pass_cnt = $sched_mc_smt_pass_cnt"
						done
						analyze_package_consolidation_result $sched_mc $work_load $sched_mc_smt_pass_cnt $sched_smt
					done
				done
			fi
		done
	fi
	if [ $hyper_threaded -eq $YES ]; then
		#Testcase to validate consolidation at core level
		sched_smt_pass_cnt=0
		: $(( TST_COUNT += 1 ))
		stress="thread"
		for repeat_test in `seq 1  10`; do
			if cpu_consolidation.py -c $sched_mc -t $sched_smt -w $work_load -s $stress; then
				: $(( sched_smt_pass_cnt += 1 ))
			fi
		done
		analyze_core_consolidation_result $sched_smt $work_load $sched_smt_pass_cnt
	fi
        # Verify ILB runs in same package as workload
        if [ $multi_socket -eq $YES -a $multi_core -eq $YES ]; then
		work_loads_list="ebizzy kernbench"	
		for sched_mc in `seq 0 $max_sched_mc`; do
			for work_load in ${work_loads_list}
			do
				: $(( TST_COUNT += 1 ))
                                ilb_test.py -c $sched_mc -w $work_load; RC=$?
				if [ $RC -eq 0 ]; then
					tst_resm TPASS "ILB & workload in same package for sched_mc=$sched_mc"
				else
					if [ $sched_mc -eq 0 ]; then
						tst_resm TPASS "ILB & workload is not in same package when sched_mc=0"
					else
						tst_resm TFAIL "ILB did not run in same package"
					fi
				fi
			done
			if [ $hyper_threaded -eq $YES ]; then
				for sched_smt in `seq 0 $max_sched_smt`; do
					for work_load in ${work_loads_list}; do
						: $(( TST_COUNT += 1 ))
						ilb_test.py -c $sched_mc -t sched_smt -w $work_load; RC=$?
 						if [ $RC -eq 0 ]; then
							tst_resm TPASS "ILB & workload in same package for sched_mc=$sched_mc"
						else
							if [ $sched_mc -eq 0 ]; then
								tst_resm TPASS "ILB & workload is not in same package when sched_mc=0"
							else
								tst_resm TFAIL "ILB did not run in same package"    
							fi
						fi
					done
				done
			fi
		done
	fi
	if [ $multi_socket -eq $YES -a $hyper_threaded -eq $YES -a $multi_core -eq $YES ]; then
		for sched_smt in `seq 0 $max_sched_smt`; do
			for work_load in ${work_loads_list}; do
				: $(( TST_COUNT += 1 ))
				ilb_test.py -t $sched_smt -w $work_load; RC=$?
				if [ $RC -eq 0 ]; then
					tst_resm TPASS "ILB & workload not load not in same package for sched_smt=$sched_smt"
				else
					if [ $sched_smt -eq 0 ]; then
						tst_resm TPASS "Its oky if ILB is not in same package when sched_smt=0"
					else
						tst_resm TFAIL "ILB did not run in same package"
					fi
				fi
			done
		done
	fi
fi

exit $RC
