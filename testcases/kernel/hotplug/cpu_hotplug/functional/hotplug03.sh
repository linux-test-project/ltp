#!/bin/sh
#
# Test Case 3
#

CASE="hotplug03"
HOTPLUG03_LOOPS=${HOTPLUG03_LOOPS:-${LOOPS}}
loop=${HOTPLUG03_LOOPS:-1}

CPU_TO_TEST=${1#cpu}
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
echo "Desc:   Do tasks get scheduled to a newly on-lined CPU?"
echo

# Verify the specified CPU exists
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
    echo "Error: CPU${CPU_TO_TEST} not found"
    echo "$CASE      FAIL: CPU${CPU_TO_TEST} not found!"
    exit_clean -1
fi

# do_clean()
#
#  Callback to be executed when script exits from a user interrupt
#  or regular program termination
#
do_clean()
{
    # Kill all the processes we started up and get rid of their pid files
    if [ -e "/var/run/hotplug4_$$.pid" ]; then
        for i in `cat /var/run/hotplug4_$$.pid`; do
            kill_pid $i
        done
        rm /var/run/hotplug4_$$.pid
    fi

    # Turn off the CPUs that were off before the test start
    until [ $cpu = 0 ];do
        offline_cpu ${on[${cpu}]}
        let "cpu = cpu - 1"
    done
}

until [ $loop = 0 ]; do 
    cpu=0
    number_of_cpus=0

    # Turns on all CPUs and saves their states
    for i in $( get_all_cpus ); do 
        online_cpu $1
        if [ $? = 0 ]; then
            let "cpu = cpu + 1"
            on[${cpu}]=$i
        fi
        let "number_of_cpus = number_of_cpus + 1"
    done

    offline_cpu ${CPU_TO_TEST}
    if [ $? != 0 ]; then
        echo "$CASE     FAIL:  CPU${CPU_TO_TEST} cannot be offlined"
        exit_clean -1
    fi

    # Start up a number of processes equal to twice the number of
    # CPUs we have.  This is to help ensure we've got enough processes
    # that at least one will migrate to the new CPU.  Store the PIDs 
    # so we can kill them later.
    let "number_of_cpus = number_of_cpus * 2"
    until [ $number_of_cpus = 0 ]; do
        $LHCS_PATH/tools/do_spin_loop > /dev/null 2>&1 &
        echo $! >> /var/run/hotplug4_$$.pid
        let "number_of_cpus = number_of_cpus - 1"
    done

    ps aux | head -n 1
    ps aux | grep do_spin_loop

    # Online the CPU
    echo "Onlining CPU ${CPU_TO_TEST}"
    online_cpu ${CPU_TO_TEST}
    RC=$?
    if [ $RC != 0 ]; then
        echo "$CASE     FAIL:  CPU${CPU_TO_TEST} cannot be onlined"
        exit_clean -1
    fi

    sleep 1

    # Verify at least one process has migrated to the new CPU
    ps -o psr -o command --no-headers -C do_spin_loop
    RC=$?
    NUM=`ps -o psr -o command --no-headers -C do_spin_loop | sed -e "s/^ *//" | cut -d' ' -f 1 | grep "^${CPU_TO_TEST}$" | wc -l`
    if [ $RC != 0 ]; then
        echo "$CASE    FAIL: No do_spin_loop processes found on any processor"
    elif [ $NUM -lt 1 ]; then
        echo "$CASE    FAIL: No do_spin_loop processes found on CPU${CPU_TO_TEST}"
    else
        echo "$CASE    PASS: $NUM do_spin_loop processes found on CPU${CPU_TO_TEST}"
    fi

    do_clean

    let "loop = loop -1"
done

exit_clean
