#!/bin/sh
#
# Test Case 2
#

CASE="hotplug02"
HOTPLUG02_LOOPS=${HOTPLUG02_LOOPS:-${LOOPS}}
loop=${HOTPLUG02_LOOPS:-1}

CPU_TO_TEST=$1
if [ -z $CPU_TO_TEST ]; then
    echo "Usage:  $0 <CPU to online>"
    exit -1
fi

# Includes:
LHCS_PATH=${LHCS_PATH:-".."}
source $LHCS_PATH/include/testsuite.fns
source $LHCS_PATH/include/hotplug.fns

echo "Name:   $CASE"
echo "Date:   `date`"
echo "Desc:   What happens to a process when its CPU is offlined?"
echo

# Start up a process that just uses CPU cycles
bash $LHCS_PATH/tools/do_spin_loop > /dev/null&
SPIN_LOOP_PID=$!

# Validate the specified CPU exists
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
    echo "Error: cpu${CPU_TO_TEST} not found"
    echo "$CASE      FAIL: cpu${CPU_TO_TEST} not found!"
    exit_clean -1
fi

# do_clean()
#
#  Callback to be executed when script exits from a user interrupt
#  or regular program termination.
#
do_clean()
{
    kill_pid ${SPIN_LOOP_PID}
}

# Validate the specified CPU is online; if not, online it
if ! cpu_is_online "${CPU_TO_TEST}" ; then
    online_cpu ${CPU_TO_TEST}
    if [ $? != 0 ]; then
        echo "Error:  CPU${CPU_TO_TEST} cannot be onlined"
        echo "$CASE     FAIL: CPU${CPU_TO_TEST} cannot be onlined line: 28"
        exit_clean -1
    fi
fi

sleep 5
until [ $loop = 0 ]; do
    # Move spin_loop.sh to the CPU to offline.
    set_affinity ${SPIN_LOOP_PID} ${CPU_TO_TEST}

    # Verify the process migrated to the CPU we intended it to go to
    offline_cpu ${CPU_TO_TEST}
    NEW_CPU=`ps --pid=${SPIN_LOOP_PID} -o psr --no-headers`
    if [ -z ${NEW_CPU} ]; then
        echo "$CASE     FAIL: PID ${SPIN_LOOP_PID} no longer running"
        exit_clean -1
    elif [ ${CPU_TO_TEST} = ${NEW_CPU} ]; then
        echo "$CASE     FAIL: process did not change from CPU ${NEW_CPU}"
        exit_clean -1
    fi
    echo "$CASE     PASS - turned off CPU ${CPU_TO_TEST}, process migrated to CPU ${NEW_CPU}"
    
    # Turn the CPU back online just to see what happens.
    online_cpu ${CPU_TO_TEST}
    let "loop = loop - 1"
done

sleep 2

exit_clean