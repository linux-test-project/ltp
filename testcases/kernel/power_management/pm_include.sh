#!/bin/bash

TMPDIR=/tmp

PASS=0
FAIL=1
NOSUPPORT=2
MISSING_FILE=3
UNTESTED=4
YES=0

cleanup() {
	if [ -f ${1} ] ; then
		rm -f ${1}
	fi
}

check_config_options() {
	if ( ! ${3} "${1}" ${2} | grep -v "#" > /dev/null ) ; then
		echo "NOSUPPORT: current system dosen't support ${1}"
	fi
}

no_of_cpus() {
	echo $(cat /proc/cpuinfo | grep processor | wc -l)
}

get_topology() {
	declare -a cpus
	declare -a phyid

	total_cpus=$(no_of_cpus)
	(( total_cpus-=1 ))
	for cpu in $(seq 0 "${total_cpus}" )
	do
		cpus[$cpu]=cpu${cpu}
		phyid[$cpu]=$(cat /sys/devices/system/cpu/cpu${cpu}/topology/physical_package_id)
	done 
	j=0
	while [ "${j}" -lt "${total_cpus}" ]
	do
		(( k = $j + 1 ))
		if [ ${phyid[$j]} -eq ${phyid[$k]} ] ; then
			echo "${cpus[$j]} -P ${cpus[$k]}" | sed -e "s/cpu//g"
		fi
		(( j+=1 ))
	done
}

check_cpufreq() {
	total_cpus=$(no_of_cpus)
	(( total_cpus-=1 ))

	for cpu in $(seq 0 "${total_cpus}" )
	do
		if [ ! -d /sys/devices/system/cpu/cpu${cpu}/cpufreq ] ; then
			echo "NOSUPPORT: cpufreq support not found please check Kernel configuration or BIOS settings"
			exit $NOSUPPORT
		fi
	done
}

get_supporting_freq() {
	cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_available_frequencies | uniq	
}

get_supporting_govr() {
	cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_available_governors | uniq
}

is_hyper_threaded() {
	siblings=`cat /proc/cpuinfo | grep siblings | uniq | cut -f2 -d':'`
	cpu_cores=`cat /proc/cpuinfo | grep "cpu cores" | uniq | cut -f2 -d':'`
	[ $siblings -gt $cpu_cores ]; return $?
}

check_input() {
	validity_check=${2:-valid}
	testfile=$3
	if [ "${validity_check}" = "invalid" ] ; then
		PASS="Testcase FAIL - Able to execute"
		FAIL="Testcase PASS - Unable to execute"
	else
		PASS="Testcase PASS"
		FAIL="Testcase FAIL"
	fi
	RC=0
	for input in ${1}
	do
		echo ${input} > ${test_file} 2>/dev/null
		return_value=$?
		output=$(cat ${test_file})
		if [ "${return_value}" = "0" -a "${input}" = "${output}" ] ; then
			echo "${0}: ${PASS}: echo ${input} > ${test_file}"
			if [ "${validity_check}" = "invalid" ] ; then
				RC=1
			fi
		else
			echo "${0}: ${FAIL}: echo ${input} > ${test_file}"
			if [ "${validity_check}" = "valid" ] ; then
				RC=1
			fi
		fi
	done
	return $RC
}

is_multi_socket() {
	no_of_sockets=`cat /sys/devices/system/cpu/cpu?/topology/physical_package_id | uniq | wc -l`
	[ $no_of_sockets -gt 1 ] ; return $?
}

is_multi_core() {
	siblings=`cat /proc/cpuinfo | grep siblings | uniq | cut -f2 -d':'`
	cpu_cores=`cat /proc/cpuinfo | grep "cpu cores" | uniq | cut -f2 -d':'`
	if [ $siblings -eq $cpu_cores ]; then
		[ $cpu_cores -gt 1 ]; return $?
	else
		: $(( num_of_cpus = siblings / cpu_cores ))
		[ $num_of_cpus -gt 1 ]; return $?
	fi
}		

is_dual_core() {
	siblings=`cat /proc/cpuinfo | grep siblings | uniq | cut -f2 -d':'`
        cpu_cores=`cat /proc/cpuinfo | grep "cpu cores" | uniq | cut -f2 -d':'`
        if [ $siblings -eq $cpu_cores ]; then
                [ $cpu_cores -eq 2 ]; return $?
        else
                : $(( num_of_cpus = siblings / cpu_cores ))
                [ $num_of_cpus -eq 2 ]; return $?
        fi
}

get_kernel_version() {
	# Get kernel minor version
	export kernel_version=`uname -r | awk -F. '{print $1"."$2"."$3}' | cut -f1 -d'-'`
}

get_valid_input() {
	kernel_version=$1
	case "$kernel_version" in
	'2.6.26' | '2.6.27' | '2.6.28')
			export valid_input="0 1" ;;
		*) export valid_input="0 1 2" ;;
	esac
}
		
analyze_result_hyperthreaded() {
	sched_mc=$1
    pass_count=$2
    sched_smt=$3

	case "$sched_mc" in
	0)
		case "$sched_smt" in
		0)
			if [ $pass_count -lt 5 ]; then
				tst_resm TPASS "cpu consolidation failed for sched_mc=\
$sched_mc & sched_smt=$sched_smt"
			else
				RC=1
				tst_resm TFAIL "cpu consolidation passed for sched_mc=\
$sched_mc & sched_smt=$sched_smt"
			fi
			;;
		*)
			if [ $pass_count -lt 5 ]; then
               	tst_resm TFAIL "cpu consolidation for sched_mc=\
$sched_mc & sched_smt=$sched_smt"
           	else
				RC=1
				tst_resm TPASS "cpu consolidation for sched_mc=\
$sched_mc & sched_smt=$sched_smt"
			fi
			;;
		esac ;;
	*)
		if [ $pass_count -lt 5 ]; then
			tst_resm TFAIL "cpu consolidation for sched_mc=\
$sched_mc & sched_smt=$sched_smt"
		else
			RC=1
			tst_resm TPASS "cpu consolidation for sched_mc=\
$sched_mc & sched_smt=$sched_smt"
		fi
		;;
	esac
}

analyze_package_consolidation_result() {
	sched_mc=$1
    pass_count=$2

	if [ $# -gt 2 ]
	then
		sched_smt=$3
	else
		sched_smt=-1
	fi

	if [ $hyper_threaded -eq $YES -a $sched_smt -gt -1 ]; then
		analyze_result_hyperthreaded $sched_mc $pass_count $sched_smt
	else
		case "$sched_mc" in
	    0)
    	    if [ $pass_count -lt 5 ]; then
        	    tst_resm TPASS "cpu consolidation failed for sched_mc=\
$sched_mc"
        	else
				RC=1
            	tst_resm TFAIL "cpu consolidation passed for sched_mc=\
$sched_mc"
        	fi ;;
    	*)
			if [ $pass_count -lt 5 ]; then
				tst_resm TFAIL "Consolidation at package level failed for \
sched_mc=$sched_mc"
			else
				tst_resm TPASS "Consolidation at package level passed for \
sched_mc=$sched_mc"
			fi	
        	;;
    	esac
	fi
}

analyze_core_consolidation_result() {
	sched_smt=$1
	pass_count=$2

	case "$sched_smt" in
	0)
		if [ $pass_count -lt 5 ]; then
			tst_resm TPASS "Consolidation at core level failed \
when sched_smt=$sched_smt"
		else
			tst_resm TFAIL "Consolidation at core level passed for \
sched_smt=$sched_smt"
		fi ;;	
	*)
		if [ $pass_count -lt 5 ]; then
			RC=1
			tst_resm TFAIL "Consolidation at core level failed for \
sched_smt=$sched_smt"
		else
			tst_resm TPASS "Consolidation at core level passed for \
sched_smt=$sched_smt"
		fi ;;
	esac
}

analyze_sched_domain_result(){
	sched_mc=$1
	result=$2
	sched_smt=$3

	if [ $hyper_threaded -eq $YES ]; then
		if [ $sched_smt ]; then
			if [ "$result" = 0 ];then
				tst_resm TPASS "sched domain test for sched_mc=$sched_mc & sched_smt=$sched_smt"
			else
				RC=1
				tst_resm TFAIL "sched domain test sched_mc=$sched_mc & sched_smt=$sched_smt"
			fi
		else
			if [ "$result" = 0 ];then
				tst_resm TPASS "sched domain test for sched_mc=$sched_mc"
			else
				RC=1
				tst_resm TFAIL "sched domain test for sched_mc=$sched_mc"		
			fi
		fi
	else
		if [ "$result" = 0 ];then
			tst_resm TPASS "sched domain test for sched_mc=$sched_mc"
		else
			RC=1
			tst_resm TFAIL "sched domain test sched_mc=$sched_mc"
		fi
	fi
}
