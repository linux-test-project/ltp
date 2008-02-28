#!/bin/bash

get_num_groups()        # Number of tasks should be >= number of cpu's (to check scheduling fairness)
{
        num_grps=$(echo "$NUM_CPUS * 1.5"|bc)   # temp variable num_grps
        int_part=`echo $num_grps | cut -d"." -f1`
        dec_part=`echo $num_grps | cut -d"." -f2`

        if [ $dec_part -gt 0 ]
        then
                NUM_GROUPS=$(echo "$int_part + 1"|bc)
        else
                NUM_GROUPS=$int_part;
        fi
}

	# Write the cleanup function
cleanup ()
{
        echo "Cleanup called";
        rm -f cpuctl_task_* 2>/dev/null
        rmdir /dev/cpuctl/group_* 2> /dev/null
        umount /dev/cpuctl 2> /dev/null
        rmdir /dev/cpuctl 2> /dev/null
        rm -f myfifo 2>/dev/null

}
        # Create /dev/cpuctl &  mount the cgroup file system with cpu controller
        #clean any group created eralier (if any)

setup ()
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
        if [ -e /dev/cpuctl/group_1 ]
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
        i=1;
        while [ "$i" -le "$NUM_GROUPS" ]
        do
                group=group_$i;
                mkdir /dev/cpuctl/$group;# 2>/dev/null
                if [ $? -ne 0 ]
                then
                        echo "ERROR: Can't create $group...Check your permissions..Exiting test";
                        cleanup;
                        exit -1;
                fi
                i=`expr $i + 1`
        done
}

