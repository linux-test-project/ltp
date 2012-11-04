#!/bin/bash
# usage ./functions.sh

#################################################################################
#  Copyright (c) International Business Machines  Corp., 2008                   #
#                                                                               #
#  This program is free software;  you can redistribute it and/or modify        #
#  it under the terms of the GNU General Public License as published by         #
#  the Free Software Foundation; either version 2 of the License, or            #
#  (at your option) any later version.                                          #
#                                                                               #
#  This program is distributed in the hope that it will be useful,              #
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of              #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See                    #
#  the GNU General Public License for more details.                             #
#                                                                               #
#  You should have received a copy of the GNU General Public License            #
#  along with this program;  if not, write to the Free Software                 #
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA      #
#                                                                               #
#################################################################################
#  Name Of File: myfunctions.sh                                                 #
#                                                                               #
#  Description: This file has functions for the setup for testing memory        #
#               controller. setup includes creating controller device,          #
#               mounting it with cgroup filesystem with option memory and       #
#               creating groups in it.                                          #
#                                                                               #
#  Functions:   setup(): creaes /dev/memctl, mounts cgroup fs on it, creates    #
#               groups in that etc.                                             #
#               setmemlimits(): Sets up memory limits for different groups      #
#               usage(): Shows the usage of this file.                          #
#               cleanup(): Does full system cleanup                             #
#                                                                               #
#  Author:       Sudhir Kumar   <skumar@linux.vnet.ibm.com>                     #
#                                                                               #
#  History:                                                                     #
#                                                                               #
#  DATE         NAME           EMAIL                         DESC               #
#                                                                               #
#  15/03/08  Sudhir Kumar <skumar@linux.vnet.ibm.com>   Created this test       #
#                                                                               #
#################################################################################

	# Write the cleanup function
cleanup ()
{
	echo "Cleanup called";
	rm -f memctl_task_* 2>/dev/null
	rmdir /dev/memctl/group* 2> /dev/null
	umount /dev/memctl 2> /dev/null
	rmdir /dev/memctl 2> /dev/null
}
	# Create /dev/memctl &  mount the cgroup file system with memory controller
	#clean any group created eralier (if any)

setup ()
{
	if [ -e /dev/memctl ]
	then
		echo "WARN:/dev/memctl already exist..overwriting";
		cleanup;
		mkdir /dev/memctl;
	else
		mkdir /dev/memctl
	fi
	mount -t cgroup -omemory cgroup /dev/memctl 2> /dev/null
	if [ $? -ne 0 ]
	then
		echo "ERROR: Could not mount cgroup filesystem on /dev/memctl..Exiting test";
		cleanup;
		exit -1;
	fi

	# Group created earlier may again be visible if not cleaned properly...so clean them
	if [ -e /dev/memctl/group_1 ]
	then
		rmdir /dev/memctl/group*
		echo "WARN: Earlier groups found and removed...";
	fi

	# Create different groups
	for i in $(seq 1 $NUM_GROUPS)
	do
		group=group_$i;
		mkdir /dev/memctl/$group;# 2>/dev/null
		if [ $? -ne 0 ]
		then
			echo "ERROR: Can't create $group...Check your permissions..Exiting test";
			cleanup;
			exit -1;
		fi
	done
}

# The usage of the script file
usage()
{
	echo "Could not start memory controller test";
	echo "usage: run_memctl_test.sh test_num";
	echo "Skipping the memory controller test...";
}

# Function to set memory limits for different groups
setmemlimits()
{
	for i in $(seq 1 $NUM_GROUPS)
	do
		limit=MEMLIMIT_GROUP_${i};
		eval limit=\$$limit;
		echo -n $limit >/dev/memctl/group_$i/memory.limit_in_bytes;
		if [ $? -ne 0 ]
		then
			echo "Error in setting the memory limits for group_$i"
			cleanup;
			exit -1;
		fi;
	done
}


