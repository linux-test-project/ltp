#!/bin/sh
#
# Test Case 4
#

CASE="hotplug04"
HOTPLUG04_LOOPS=${HOTPLUG04_LOOPS:-${LOOPS}}
loop=${HOTPLUG04_LOOPS:-1}

# Includes:
LHCS_PATH=${LHCS_PATH:-".."}
source $LHCS_PATH/include/testsuite.fns
source $LHCS_PATH/include/hotplug.fns

echo "Name:   $CASE"
echo "Date:   `date`"
echo "Desc:   Does it prevent us from offlining the last CPU?"
echo

cpu=0
until [ $loop = 0 ]; do
    cpustate=1

    # Online all the CPUs' keep track of which were already on
    for i in $( get_all_cpus ); do
        online_cpu $i
        RC=$?
        if [ $RC != 0 ]; then
            let "cpu = cpu + 1"
            on[${cpu}]=$i
            echo "${on[${cpu}]}"
        fi
        if [ $RC = 0 -a "$i" = "cpu0" ]; then
            cpustate=0
        fi
    done

    # Now offline all the CPUs
    for i in $( get_all_cpus ); do
        offline_cpu $i
        RC=$?
        if [ $RC = 1 ]; then
            if [ "$i" != "cpu0" ]; then
                echo "$CASE      FAIL: Could not shutdown $i (Maybe:  No Hotplug available)"
            else
                echo "$CASE      PASS: Could not shutdown $i"
            fi
        fi
    done

    # Online the ones that were on initially
    until [ $cpu = 0 ]; do
        online_cpu ${on[${cpu}]}
        let "cpu = cpu - 1"
    done

    # Return CPU 0 to its initial state
    if [ $cpustate = 1 ]; then
        online_cpu 0
    else 
        offline_cpu 0
    fi

    let "loop = loop -1"
done

exit_clean