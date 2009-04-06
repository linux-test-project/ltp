#!/bin/bash

TMPDIR=/tmp

PASS=0
FAIL=1
NOSUPPORT=2
MISSING_FILE=3
UNTESTED=4

function cleanup() {
	if [ -f ${1} ] ; then
		rm -f ${1}
	fi
}
function check_config_options() {
	if ( ! ${3} "${1}" ${2} | grep -v "#" > /dev/null ) ; then
		echo "NOSUPPORT: current system dosen't support ${1}"
	fi
}
function no_of_cpus() {
	echo $(cat /proc/cpuinfo | grep processor | wc -l)
}
function get_topology() {
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
function check_cpufreq() {
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
function get_supporting_freq() {
	cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_available_frequencies | uniq	
}
function get_supporting_govr() {
	cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_available_governors | uniq
}

function is_hyper_threaded() {
	siblings=`cat /proc/cpuinfo | grep siblings | uniq | cut -f2 -d':'`
	cpu_cores=`cat /proc/cpuinfo | grep "cpu cores" | uniq | cut -f2 -d':'`
	if [ $siblings -eq $cpu_cores ]; then
		echo 0
	else
		echo 1
	fi
}

function check_input() {
	validity_check=${2:-valid}
	testfile=$3
	if [ "${validity_check}" = "invalid" ] ; then
		PASS="Testcase FAIL - Able to execute"
		FAIL="Testcase PASS - Unable to execute"
	else
		PASS="Testcase PASS"
		FAIL="Testcase FAIL"
	fi
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
 
