#!/bin/bash
#usage ./test_controllers.sh
##################################################################################
#  Copyright (c) International Business Machines  Corp., 2007                    #
#                                                                                #
#  This program is free software;  you can redistribute it and/or modify         #
#  it under the terms of the GNU General Public License as published by          #
#  the Free Software Foundation; either version 2 of the License, or             #
#  (at your option) any later version.                                           #
#                                                                                #
#  This program is distributed in the hope that it will be useful,               #
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of               #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                     #
#  the GNU General Public License for more details.                              #
#                                                                                #
#  You should have received a copy of the GNU General Public License             #
#  along with this program;  if not, write to the Free Software                  #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA       #
#                                                                                #
##################################################################################
# Name Of File: test_controllers.sh                                              #
#                                                                                #
# Description:                                                                   #
#               This file runs the tests for resource management ie to test cpu  #
#               controller and memory controller. (for now cpu controller only)  #
#                                                                                #
# Author:       Sudhir Kumar    <sudhirkumarmalik@In.ibm.com>                    #
#                                                                                #
# History:                                                                       #
#                                                                                #
#  DATE        NAME            EMAIL                         DESC                #
#                                                                                #
#  20/12/07  Sudhir Kumar <sudhirkumarmalik@in.ibm.com>   Created this test      #
#                                                                                #
##################################################################################

if [ -f /proc/cgroups ]
then
	CPU_CONTROLLER=`grep -w cpu /proc/cgroups | cut -f1`;
	if [ "$CPU_CONTROLLER" = "cpu" ]
	then
		$LTPROOT/testcases/bin/run_cpuctl_test.sh 1;
		$LTPROOT/testcases/bin/run_cpuctl_test.sh 2;
		$LTPROOT/testcases/bin/run_cpuctl_test.sh 3;
	else
		echo "CONTROLLERS TESTCASES: WARNING";
		echo "Kernel does not support for cpu controller";
		echo "Skipping all controllers testcases....";
	fi
else
	echo "CONTROLLERS TESTCASES: WARNING"
	echo "Kernel does not support for control groups";
	echo "Skipping all controllers testcases....";
fi

exit 0;
