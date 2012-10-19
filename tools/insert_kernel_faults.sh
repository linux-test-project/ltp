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
#									      ##
# File :        insert_kernel_faults.sh					      ##
#									      ##
# Usage:	insert_kernel_faults.sh <fault_percentage>		      ##
#									      ##
# Description:  This is a simple script that inserts faults at various	      ##
#		subsystems of the kernel. Please refer to the ltp/README      ##
#		for the various kernel CONFIG options needed to exploit	      ##
#		all those features					      ##
#									      ##
# Author:	Subrata Modak <subrata@linux.vnet.ibm.com>		      ##
#									      ##
# History:      Aug 11 2009 - Created - Subrata Modak.			      ##
#		Aug 17 2009 - Changed the debugfs mount point - Subrata Modak.##
################################################################################

if [ -z "$1" ]
	then
	#Check if Useage has been proper
	echo "Usage: $0 <fault_percentage>"
	exit 1
fi

#These are the types of Subsystems where fault will be injected
#Make sure debugfs has been mounted
for FAILTYPE in fail_io_timeout fail_make_request fail_page_alloc failslab
do
	echo $1 > /sys/kernel/debug/$FAILTYPE/probability
	echo 100 > /sys/kernel/debug/$FAILTYPE/interval
	echo -1 > /sys/kernel/debug/$FAILTYPE/times
	echo 0 > /sys/kernel/debug/$FAILTYPE/space
done

