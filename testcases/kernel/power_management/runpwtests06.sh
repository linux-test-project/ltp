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

export TCID="Power_Management06"
export TST_TOTAL=1

. test.sh
. pm_include.sh

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

test_timer_migration() {
	valid_input="0 1"
	invalid_input="3 4 5 6 7 8 a abcefg x1999 xffff -1 -00000
	2000000000000000000000000000000000000000000000000000000000000000000000
	ox324 -0xfffffffffffffffffffff"
	test_file="/proc/sys/kernel/timer_migration"
	if [ ! -f ${test_file} ] ; then
		tst_brkm TBROK "MISSING_FILE: missing file ${test_file}"
	fi

	RC=0
	echo "${0}: ---Valid test cases---"
	check_input "${valid_input}" valid $test_file
	RC=$?
	echo "${0}: ---Invalid test cases---"
	check_input "${invalid_input}" invalid $test_file
	RC=$(( RC | $? ))
	return $RC
}

# Checking test environment
check_arch

timer_migr_support_compatible=0

if [ $timer_migr_support_compatible -eq 1 ]; then
	tst_brkm TCONF "Kernel version does not support Timer migration"
else
	if [ ! -f /proc/sys/kernel/timer_migration ]; then
		tst_brkm TBROK "Timer migration interface missing"
	fi
fi

if test_timer_migration ; then
	tst_resm TPASS "Timer Migration interface test"
else
	tst_resm TFAIL "Timer migration interface test"
fi

tst_exit
