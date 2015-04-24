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

# Checking test environment
check_kervel_arch

tst_kvercmp 2 6 31; rc=$?
if [ $rc -eq 1 -o $rc -eq 2 ] ; then
	timer_migr_support_compatible=0
else
	timer_migr_support_compatible=1
fi

if [ $timer_migr_support_compatible -eq 1 ]; then
	tst_brkm TCONF "Kernel version does not support Timer migration"
else
	if [ ! -f /proc/sys/kernel/timer_migration ]; then
		tst_brkm TBROK "Timer migration interface missing"
	fi
fi

if test_timer_migration.sh; then
	tst_resm TPASS "Timer Migration interface test"
else
	tst_resm TFAIL "Timer migration interface test"
fi

tst_exit
