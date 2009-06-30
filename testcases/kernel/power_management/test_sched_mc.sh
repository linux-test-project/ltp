#!/bin/bash

. pm_include.sh

get_kernel_version
get_valid_input $kernel_version

invalid_input="3 4 5 6 7 8 a abcefg x1999 xffff -1 -00000
200000000000000000000000000000000000000000000000000000000000000000000000000000
ox324 -0xfffffffffffffffffffff"
test_file="/sys/devices/system/cpu/sched_mc_power_savings"
if [ ! -f ${test_file} ] ; then
	echo "MISSING_FILE: missing file ${test_file}"
	exit $MISSING_FILE
fi

RC=0
echo "${0}: ---Valid test cases---"
check_input "${valid_input}" valid $test_file 
RC=$?
echo "${0}: ---Invalid test cases---"
check_input "${invalid_input}" invalid $test_file
RC=$(( RC | $? ))
exit $RC
