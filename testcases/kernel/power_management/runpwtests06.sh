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
