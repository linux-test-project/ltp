#! /bin/sh

###########################################################################
##                                                                       ##
## Copyright (c) 2015, Red Hat Inc.                                      ##
##                                                                       ##
## This program is free software: you can redistribute it and/or modify  ##
## it under the terms of the GNU General Public License as published by  ##
## the Free Software Foundation, either version 3 of the License, or     ##
## (at your option) any later version.                                   ##
##                                                                       ##
## This program is distributed in the hope that it will be useful,       ##
## but WITHOUT ANY WARRANTY; without even the implied warranty of        ##
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          ##
## GNU General Public License for more details.                          ##
##                                                                       ##
## You should have received a copy of the GNU General Public License     ##
## along with this program. If not, see <http://www.gnu.org/licenses/>.  ##
##                                                                       ##
## Author: Li Wang <liwang@redhat.com>                                   ##
##                                                                       ##
###########################################################################
##                                                                       ##
## Summary:  panic while using userstacktrace                            ##
##                                                                       ##
## BUG: unable to handle kernel paging request at 00000000417683c0       ##
##      IP: [<ffffffff8105c834>] update_curr+0x124/0x1e0                 ##
##      PGD 41a796067 PUD 0                                              ##
##      Thread overran stack, or stack corrupted                         ##
##      Oops: 0000 [#1] SMP                                              ##
##      last sysfs file: ../system/cpu/cpu15/cache/index2/shared_cpu_map ##
##                                                                       ##
## The bug was fixed by:                                                 ##
##      1dbd195 (tracing: Fix preempt count leak)                        ##
##                                                                       ##
###########################################################################

export TCID="ftrace_regression01"
export TST_TOTAL=1

. ftrace_lib.sh

LOOP=10

TSTACK_TRACE_PATH="/proc/sys/kernel/stack_tracer_enabled"
EXC_PAGE_FAULT_ENABLE="$TRACING_PATH/events/exceptions/page_fault_kernel/enable"
MM_PAGE_FAULT_ENABLE="$TRACING_PATH/events/kmem/mm_kernel_pagefault/enable"

ftrace_userstacktrace_test()
{
	if [ ! -e "$TSTACK_TRACE_PATH" ]; then
		tst_brkm TCONF "Stack Tracer is not cofigured in This kernel"
	fi

	for i in $(seq $LOOP); do
		echo 1 >  $TSTACK_TRACE_PATH
		echo userstacktrace > $TRACING_PATH/trace_options
		grep -q "^userstacktrace"  $TRACING_PATH/trace_options
		if [ $? -ne 0 ]; then
			tst_brkm TBROK "Failed to set userstacktrace"
		fi

		if [ -f "$EXC_PAGE_FAULT_ENABLE" ]; then
			exc_page_fault_enable=`cat $EXC_PAGE_FAULT_ENABLE`
			echo 1 > $EXC_PAGE_FAULT_ENABLE
		else
			mm_page_fault_enable=`cat $MM_PAGE_FAULT_ENABLE`
			echo 1 > $MM_PAGE_FAULT_ENABLE
		fi
	done

	if [ -f "$EXC_PAGE_FAULT_ENABLE" ]; then
		echo "$exc_page_fault_enable" > $EXC_PAGE_FAULT_ENABLE
	else
		echo "$mm_page_fault_enable" > $MM_PAGE_FAULT_ENABLE
	fi

	tst_resm TPASS "Finished running the test"
}

ftrace_userstacktrace_test

tst_exit
