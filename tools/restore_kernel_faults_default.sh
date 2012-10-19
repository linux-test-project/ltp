#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2009                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and/or modify      ##
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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
################################################################################
#                                                                             ##
# File :        restore_kernel_faults_default.sh			      ##
#                                                                             ##
# Usage:        restore_kernel_faults_default.sh	                      ##
#                                                                             ##
# Description:  This is a simple script that will restore the /debugfs/fail*  ##
#		entries to their default values				      ##
#                                                                             ##
# Author:       Subrata Modak <subrata@linux.vnet.ibm.com>                    ##
#                                                                             ##
# History:      Aug 11 2009 - Created - Subrata Modak.                        ##
#		Aug 17 2009 - Changed debugfs mount point - Subrata Modak.    ##
################################################################################

echo  0 > /sys/kernel/debug/fail_io_timeout/reject-end
echo  0 > /sys/kernel/debug/fail_io_timeout/reject-start
echo  4294967295 > /sys/kernel/debug/fail_io_timeout/require-end
echo  0 > /sys/kernel/debug/fail_io_timeout/require-start
echo  32 > /sys/kernel/debug/fail_io_timeout/stacktrace-depth
echo  N > /sys/kernel/debug/fail_io_timeout/task-filter
echo  2 > /sys/kernel/debug/fail_io_timeout/verbose
echo  0 > /sys/kernel/debug/fail_io_timeout/space
echo  1 > /sys/kernel/debug/fail_io_timeout/times
echo  1 > /sys/kernel/debug/fail_io_timeout/interval
echo  0 > /sys/kernel/debug/fail_io_timeout/probability

echo  0 > /sys/kernel/debug/fail_make_request/reject-end
echo  0 > /sys/kernel/debug/fail_make_request/reject-start
echo  4294967295 > /sys/kernel/debug/fail_make_request/require-end
echo  0 > /sys/kernel/debug/fail_make_request/require-start
echo  32 > /sys/kernel/debug/fail_make_request/stacktrace-depth
echo  N > /sys/kernel/debug/fail_make_request/task-filter
echo  2 > /sys/kernel/debug/fail_make_request/verbose
echo  0 > /sys/kernel/debug/fail_make_request/space
echo  1 > /sys/kernel/debug/fail_make_request/times
echo  1 > /sys/kernel/debug/fail_make_request/interval
echo  0 > /sys/kernel/debug/fail_make_request/probability

echo  1 > /sys/kernel/debug/fail_page_alloc/min-order
echo  Y > /sys/kernel/debug/fail_page_alloc/ignore-gfp-highmem
echo  Y > /sys/kernel/debug/fail_page_alloc/ignore-gfp-wait
echo  0 > /sys/kernel/debug/fail_page_alloc/reject-end
echo  0 > /sys/kernel/debug/fail_page_alloc/reject-start
echo  4294967295 > /sys/kernel/debug/fail_page_alloc/require-end
echo  0 > /sys/kernel/debug/fail_page_alloc/require-start
echo  32 > /sys/kernel/debug/fail_page_alloc/stacktrace-depth
echo  N > /sys/kernel/debug/fail_page_alloc/task-filter
echo  2 > /sys/kernel/debug/fail_page_alloc/verbose
echo  0 > /sys/kernel/debug/fail_page_alloc/space
echo  1 > /sys/kernel/debug/fail_page_alloc/times
echo  1 > /sys/kernel/debug/fail_page_alloc/interval
echo  0 > /sys/kernel/debug/fail_page_alloc/probability

echo  Y > /sys/kernel/debug/failslab/ignore-gfp-wait
echo  0 > /sys/kernel/debug/failslab/reject-end
echo  0 > /sys/kernel/debug/failslab/reject-start
echo  4294967295 > /sys/kernel/debug/failslab/require-end
echo  0 > /sys/kernel/debug/failslab/require-start
echo  32 > /sys/kernel/debug/failslab/stacktrace-depth
echo  N > /sys/kernel/debug/failslab/task-filter
echo  2 > /sys/kernel/debug/failslab/verbose
echo  0 > /sys/kernel/debug/failslab/space
echo  1 > /sys/kernel/debug/failslab/times
echo  1 > /sys/kernel/debug/failslab/interval
echo  0 > /sys/kernel/debug/failslab/probability

