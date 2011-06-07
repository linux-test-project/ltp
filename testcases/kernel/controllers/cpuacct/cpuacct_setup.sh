#!/bin/bash
# usage ./cpuacct_setup.sh

################################################################################
#  Copyright (c) International Business Machines  Corp., 2009                  #
#                                                                              #
#  This program is free software;  you can redistribute it and/or modify       #
#  it under the terms of the GNU General Public License as published by        #
#  the Free Software Foundation; either version 2 of the License, or           #
#  (at your option) any later version.                                         #
#                                                                              #
#  This program is distributed in the hope that it will be useful,             #
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of             #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                   #
#  the GNU General Public License for more details.                            #
#                                                                              #
#  You should have received a copy of the GNU General Public License           #
#  along with this program;  if not, write to the Free Software                #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA     #
#                                                                              #
################################################################################
################################################################################
# Name Of File: setup.sh                                        	       #
#                                                                              #
#  Description: This file has functions for the setup for testing cpu account  #
#               controller. setup includes creating controller device,         #
#               mounting it with cgroup filesystem with option cpu account     #
#		and creating groups in it.                                     #
#                                                                              #
#  Functions:   setup(): creaes /dev/cpuacct, mounts cgroup fs on it, creates  #
#               groups in that etc.                                            #
#               usage(): Shows the usage of this file.                         #
#               cleanup(): Does full system cleanup                            #
#                                                                              #
# Precaution:   Avoid system use by other applications/users to get fair and   #
#               appropriate results (avoid unnecessary killing of applicatio)  #
#                                                                              #
# Author:       Rajasekhar Duddu   <rajduddu@in.ibm.com>                       #
#                                                                              #
# History:                                                                     #
#                                                                              #
#  DATE         NAME           EMAIL                         DESC              #
#                                                                              #
#  14/07/09  Rajasekhar D    <rajduddu@in.ibm.com>        Created this test    #
#                                                                              #
################################################################################

# The cleanup function
cleanup ()
{
	echo "Cleanup called"
	rm -rf txt*
	rmdir /dev/cpuacct/group*/group* 2> /dev/null
	rmdir /dev/cpuacct/group* 2> /dev/null
	umount /dev/cpuacct/ 2> /dev/null
	rmdir /dev/cpuacct 2> /dev/null
	rm -rf tmp2 2> /dev/null
}
task_kill ()
{
	for i in `ps -e | grep cpuacct_task | awk '{print $1}'`
	do
		kill -SIGUSR1 $i
	done
	sleep 1
	rm -rf txt* 2> /dev/null
}
#Create /dev/cpuacct & mount the cgroup file system with
#cpu accounting controller

#clean any group created eralier (if any)

setup ()
{
	if [ -e /dev/cpuacct ]
	then
		echo "WARN:/dev/cpuacct already exist..overwriting"
		rmdir /dev/cpuacct/group*/group* 2> /dev/null
	        rmdir /dev/cpuacct/group* 2> /dev/null
        	umount /dev/cpuacct/ 2> /dev/null
	        rmdir /dev/cpuacct 2> /dev/null

		mkdir /dev/cpuacct
	else
		mkdir /dev/cpuacct
	fi
	mount -t cgroup -ocpuacct none /dev/cpuacct 2> /dev/null
	if [ $? -ne 0 ]
	then
		echo "TFAIL: Could not mount cgroup filesystem"
		echo "Exiting test"
		cleanup
		exit -1
	fi

	# Group created earlier may again be visible if not cleaned properly.
	#so clean them
	if [ -e /dev/cpuacct/group_1 ]
	then
		rmdir /dev/cpuacct/group*/group* 2> /dev/null
		rmdir /dev/cpuacct/group* 2> /dev/null
		echo "WARN: Earlier groups found and removed...";
	fi

}

# The usage of the script file
usage()
{
	echo "Could not start cpu account controller test";
	echo "usage: run_cpuacct_test.sh $TEST_NUM ";
	echo "Skipping the cpu account controller test...";
}
