#!/bin/sh
#
# Test Case 6 - top
#

CASE="hotplug06.top"
HOTPLUG06_LOOPS=${HOTPLUG06_LOOPS:-${LOOPS}}
loop=${HOTPLUG06_LOOPS:-1}

CPU_TO_TEST=$1
if [ -z $CPU_TO_TEST ]; then
    echo "Usage:  $0 <CPU to offline>"
    exit_clean -1
fi

# Includes:
LHCS_PATH=${LHCS_PATH:-".."}
source $LHCS_PATH/include/testsuite.fns
source $LHCS_PATH/include/hotplug.fns

echo "Name:   $CASE"
echo "Date:   `date`"
echo "Desc:   Does top work properly when CPU hotplug events occur?"
echo

# Verify that the specified CPU is available
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
    echo "Error: CPU${CPU_TO_TEST} not found"
    echo "$CASE      FAIL: CPU${CPU_TO_TEST} not found!"
    exit_clean -1
fi

# Check that the specified CPU is online; if not, online it
if ! cpu_is_online "${CPU_TO_TEST}" ; then
    online_cpu ${CPU_TO_TEST}
    if [ $? != 0 ]; then
        echo "Error:  CPU${CPU_TO_TEST} cannot be onlined"
        echo "$CASE     FAIL: CPU${CPU_TO_TEST} cannot be onlined line: 30"
        exit_clean -1
    fi
fi

do_clean()
{
    if pid_is_valid ${TOP_PID} ; then
        kill_pid ${TOP_PID}
    fi
    online_cpu ${CPU_TO_TEST}
}

until [ $loop = 0 ]; do
    # Start up top and give it a little time to run
    top -b -d 00.10 > /dev/null 2>&1 & 
    TOP_PID=$!
    sleep 1

    # Now offline the CPU
    offline_cpu ${CPU_TO_TEST}
    if [ $? != 0 ]; then
        echo "Error:  CPU${CPU_TO_TEST} cannot be onlined"
        echo "$CASE     FAIL: CPU${CPU_TO_TEST} cannot be onlined line: 42"
        exit_clean -1
    fi

    # Wait a little time for top to notice the CPU is gone
    sleep 1

    # Check that top hasn't crashed
    pid_is_valid ${TOP_PID}
    if [ $? ]; then
	echo "$CASE    PASS: PID ${TOP_PID} still running."
    	online_cpu ${CPU_TO_TEST}
	kill_pid ${TOP_PID}
    else
        echo "$CASE     FAIL: PID ${TOP_PID} no longer running"
        exit_clean -1
    fi

    let "loop = loop - 1"

done

exit_clean
