#!/bin/bash

. pm_include.sh

valid_input="0 1"
invalid_input="a 2 abcefg x1999 xffff -1 -00000 200000000000000000000000000000000000000000000000000000000000000000000000000000 ox324 -0xfffffffffffffffffffff"
test_file="/sys/devices/system/cpu/sched_mc_power_savings"
if [ ! -f ${test_file} ] ; then
	echo "MISSING_FILE: missing file ${test_file}"
	exit $MISSING_FILE
fi

function check_input() {
	validity_check=${2:-valid}
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
}
RC=0
echo "${0}: ---Valid test cases---"
check_input "${valid_input}"
echo "${0}: ---Invalid test cases---"
check_input "${invalid_input}" invalid
#return $RC
