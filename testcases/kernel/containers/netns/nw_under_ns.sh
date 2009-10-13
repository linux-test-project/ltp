#!/bin/bash

#############################################################################
#                                                                           #
# Copyright (c) International Business Machines  Corp., 2008                #
#                                                                           #
# This program is free software;  you can redistribute it and#or modify     #
# it under the terms of the GNU General Public License as published by      #
# the Free Software Foundation; either version 2 of the License, or         #
# (at your option) any later version.                                       #
#                                                                           #
# This program is distributed in the hope that it will be useful, but       #
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY#
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License  #
# for more details.                                                         #
#                                                                           #
# You should have received a copy of the GNU General Public License         #
# along with this program;  if not, write to the Free Software              #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA   #
#                                                                           #
# Author:      Sudhir Kumar <skumar@linux.vnet.ibm.com>                     #
#############################################################################

# this script is used to run all the testcases for networks under network
# namespace. This script is called by a separate command file nw_under_ns
# The testcases are grouped as per the requirement of the particular
# testcase.

echo "*****************************************************"
echo "Running network testcases under containers..."

if ! create_container; then
	echo "some of the network testcases under netns failed"
	exit 1;
fi
