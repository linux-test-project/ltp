#!/bin/sh
#
# Test Case 6 - sar
#

CASE="hotplug06.sar"
HOTPLUG06_LOOPS=${HOTPLUG06_LOOPS:-${LOOPS}}
loop=${HOTPLUG06_LOOPS:-1}

CPU_TO_TEST=$1
if [ -z $CPU_TO_TEST ]; then
    echo "Usage:  $0 <CPU to offline>"
    exit -1
fi

# Includes:
LHCS_PATH=${LHCS_PATH:-".."}
source $LHCS_PATH/include/testsuite.fns
source $LHCS_PATH/include/hotplug.fns

echo "Name:   $CASE"
echo "Date:   `date`"
echo "Desc:   Does sar behave properly during CPU hotplug events?"
echo

# Verify the specified CPU is available
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
    echo "Error: CPU${CPU_TO_TEST} not found"
    echo "$CASE      FAIL: CPU${CPU_TO_TEST} not found!"
    exit_clean -1
fi

# Check that the specified CPU is offline; if not, offline it
if cpu_is_online "${CPU_TO_TEST}" ; then
    offline_cpu ${CPU_TO_TEST}
    if [ $? != 0 ]; then
        echo "Error:  CPU${CPU_TO_TEST} cannot be offlined"
        echo "$CASE     FAIL: CPU${CPU_TO_TEST} cannot be offlined line: 30"
        exit_clean -1
    fi
fi

do_clean()
{
    kill_pid ${SAR_PID}
}

until [ $loop = 0 ]; do
    # Start up SAR and give it a couple cycles to run
    sar -P ALL 1 0 > /tmp/log_$$ &
    sleep 2
    SAR_PID=$!
    
    # Verify that SAR has correctly listed the missing CPU as 'nan'
    cat /tmp/log_$$ | grep -i nan > /dev/null
    while [ $? != 0 ]; do 
        echo "$CASE    FAIL: CPU${CPU_TO_TEST} Not Found on SAR!"
        exit_clean -1
    done
    time=`date +%X`
    sleep .5

    # Verify that at least some of the CPUs are offline
    NUMBER_CPU_OFF=`cat /tmp/log_$$ | grep "${time}" | grep -i nan | wc -l`
    if [ ${NUMBER_CPU_OFF} = 0 ]; then
        echo "$CASE    FAIL: No CPUs found to be off"
        exit_clean -1
    fi

    # Online the CPU
    online_cpu ${CPU_TO_TEST}
    if [ $? != 0 ]; then
        echo "Error:  CPU${CPU_TO_TEST} cannot be onlined"
        echo "$CASE     FAIL: CPU${CPU_TO_TEST} cannot be onlined line: 60"
        exit_clean -1
    fi
    
    sleep 1
    time=`date +%T`
    sleep .5

    # Check that SAR registered the change in CPU online/offline states
    NEW_NUMBER_CPU_OFF=`cat /tmp/log_$$ | grep ${time} | grep -i nan | wc -l`
    let "NUMBER_CPU_OFF = NUMBER_CPU_OFF - 1"
    if [ $NUMBER_CPU_OFF = $NEW_NUMBER_CPU_OFF ]; then
        echo "$CASE     PASS: CPU was found after turned on."
    else
        echo "$CASE     FAIL: No Change in number of offline CPUs was found."
    fi

    if [ $? != 0 ]; then
        echo "Error:  CPU${CPU_TO_TEST} cannot be onlined"
        echo "$CASE     FAIL: CPU${CPU_TO_TEST} cannot be onlined line: 60"
        exit_clean -1
    fi

    let "loop = loop - 1"
done

exit_clean
