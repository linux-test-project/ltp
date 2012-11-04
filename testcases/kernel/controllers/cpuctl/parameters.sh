#!/bin/bash
# usage ./parameters.sh

#################################################################################
#  Copyright (c) International Business Machines  Corp., 2007                   #
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
# Name Of File: parameters.sh                                                   #
#                                                                               #
# Description: 	This file has functions for the setup for testing cpucontroller #
#               setup includes creating controller device, mounting it with     #
#               cgroup filesystem with option cpu and creating groups in it.    #
#                                                                               #
# Functions:    get_num_groups(): decides number of groups based on num of cpus	#
#               setup(): creaes /dev/cpuctl, mounts cgroup fs on it, creates 	#
#               groups in that, creates fifo to fire tasks at one time.         #
#               cleanup(): Does full system cleanup                             #
#                                                                               #
# Author:       Sudhir Kumar   <skumar@linux.vnet.ibm.com>                      #
#                                                                               #
# History:                                                                      #
#                                                                               #
#  DATE         NAME           EMAIL                         DESC               #
#                                                                               #
#  20/12/07  Sudhir Kumar <skumar@linux.vnet.ibm.com>   Created this test       #
#                                                                               #
#################################################################################


set_def_group() #default group spinning a task to create ideal scenario
{
	[ -d /dev/cpuctl/group_def ] || mkdir /dev/cpuctl/group_def;
	if [ $? -ne 0 ]
	then
		echo "ERROR: Can't create default group... "
			"Check your permissions..Exiting test";
		cleanup;
		exit -1;
	fi
	# Migrate all the running tasks to this group
	# rt tasks require a finite value to cpu.rt_runtime_us
	echo 10000 > /dev/cpuctl/group_def/cpu.rt_runtime_us;
	for task in `cat /dev/cpuctl/tasks`; do
		echo $task > /dev/cpuctl/group_def/tasks 2>/dev/null 1>&2;
	done
}

get_num_groups()        # Number of tasks should be >= number of cpu's (to check scheduling fairness)
{
        NUM_GROUPS=$(( (NUM_CPUS*3 + 1)/2 ))
}

	# Write the cleanup function
cleanup ()
{
        echo "Cleanup called";
	killall cpuctl_def_task01 1>/dev/null 2>&1;
	killall cpuctl_def_task02 1>/dev/null 2>&1;
	killall cpuctl_task_* 1>/dev/null 2>&1;
	sleep 1
        rm -f cpuctl_task_* 2>/dev/null
	for task in `cat /dev/cpuctl/group_def/tasks`; do
		echo $task > /dev/cpuctl/tasks 2>/dev/null 1>&2;
	done
        rmdir /dev/cpuctl/group* 2> /dev/null
        umount /dev/cpuctl 2> /dev/null
        rmdir /dev/cpuctl 2> /dev/null
        rm -f myfifo 2>/dev/null

}
        # Create /dev/cpuctl &  mount the cgroup file system with cpu controller
        #clean any group created eralier (if any)

do_setup ()
{
        if [ -e /dev/cpuctl ]
        then
                echo "WARN:/dev/cpuctl already exist..overwriting"; # or a warning ?
                cleanup;
                mkdir /dev/cpuctl;
        else
                mkdir /dev/cpuctl
        fi
        mount -t cgroup -ocpu cgroup /dev/cpuctl 2> /dev/null
        if [ $? -ne 0 ]
        then
                echo "ERROR: Could not mount cgroup filesystem on /dev/cpuctl..Exiting test";
                cleanup;
                exit -1;
        fi

        # Group created earlier may again be visible if not cleaned properly...so clean them
	groups=/dev/cpuctl/group*
        if [ -z "$groups" ]
        then
                rmdir /dev/cpuctl/group*
                echo "WARN: Earlier groups found and removed...";
        fi

        #Create a fifo to make all tasks wait on it
        mkfifo myfifo 2> /dev/null;
        if [ $? -ne 0 ]
        then
                echo "ERROR: Can't create fifo...Check your permissions..Exiting test";
                cleanup;
                exit -1;
        fi

        # Create different groups
        for i in $(seq 1 $NUM_GROUPS)
        do
                group=group_$i;
                mkdir /dev/cpuctl/$group;# 2>/dev/null
                if [ $? -ne 0 ]
                then
                        echo "ERROR: Can't create $group...Check your permissions..Exiting test";
                        cleanup;
                        exit -1;
                fi
        done
}

