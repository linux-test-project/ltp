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

# The script to be run in the parent network namespace
# Add the code as per the requirement of different existing
# network testcases

echo "This is parent process....."
service xinetd restart;
sleep 5;
