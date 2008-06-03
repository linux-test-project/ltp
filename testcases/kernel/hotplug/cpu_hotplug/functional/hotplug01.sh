#!/bin/bash
#
# Test Case 1
#
# Based on script by Ashok Raj <ashok.raj@intel.com>
# Modified by Mark D and Bryce, Aug '05.

CASE="hotplug01"
HOTPLUG01_LOOPS=${HOTPLUG01_LOOPS:-${LOOPS}}
loop=${HOTPLUG01_LOOPS:-1}

CPU_TO_TEST=${1#cpu}
if [ -z "${CPU_TO_TEST}" ]; then
    echo "Usage:  $0 <CPU to online>"
    exit -1
fi

# Includes:
LHCS_PATH=${LHCS_PATH:-".."}
source $LHCS_PATH/include/testsuite.fns
source $LHCS_PATH/include/hotplug.fns

echo "Name:   $CASE"
echo "Date:   `date`"
echo "Desc:   What happens to disk controller interrupts when offlining CPUs?"
echo

# Time delay after an online of cpu
TM_ONLINE=${HOTPLUG01_TM_ONLINE:-1} 

# Time delay after offline of cpu
TM_OFFLINE=${HOTPLUG01_TM_OFFLINE:-1} 

# Time delay before start of entire new cycle.
TM_DLY=${HOTPLUG01_TM_DLY:-6}    

# Validate the specified CPU exists
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
    echo "Error: cpu${CPU_TO_TEST} not found"
    echo "$CASE      FAIL: cpu${CPU_TO_TEST} not found!"
    exit_clean -1
fi

CPU_COUNT=0
cpustate=1

if ! cpu_is_online "${CPU_TO_TEST}" ; then
    online_cpu ${CPU_TO_TEST}
    if [ $? != 0 ]; then
        echo "Error:  Could not online cpu $CPU_TO_TEST"
        exit_clean -1
    fi
    cpustate=0
fi

# do_clean()
#
#  Callback to be executed when script exits from a user interrupt
#  or regular program termination
#
do_clean()
{
    kill_pid ${WRL_ID}

    # Turns off the cpus that were off before the test start
    echo "Return to previous state.  CPU count = ${CPU_COUNT}"
    until [ $CPU_COUNT = 0 ]; do    
    	echo "CPU = $CPU_COUNT @on = ${OFFLINE_CPU[${CPU_COUNT}]}"
        offline_cpu ${OFFLINE_CPU[${CPU_COUNT}]}
        let "CPU_COUNT = CPU_COUNT - 1"
    done			
    if [ ${cpustate} = 1 ]; then
        online_cpu ${CPU_TO_TEST}
    else
        offline_cpu ${CPU_TO_TEST}
    fi
}


# do_offline(CPU)
#
#  Migrates some irq's onto the CPU, then offlines it
#
do_offline()
{
    CPU=${1#cpu}
    # Migrate some irq's this way first.
    IRQS=`get_all_irqs`
    migrate_irq ${CPU} ${IRQS}
    offline_cpu ${CPU}
    if [ $? != 0 ]; then
        if [ "$CPU" != "0" ]; then
            let "CPU_COUNT = CPU_COUNT + 1"
            OFFLINE_CPU[${CPU_COUNT}]=$1
        fi
        return 1
    fi
    return 0
}


# do_online(CPU)
#
#  Onlines the CPU and then sets the smp_affinity of all IRQs to 
#  this CPU.
#
do_online()
{
    CPU=${1#cpu}
    online_cpu ${CPU}
    if [ $? != 0 ]; then
        return 1
    fi
    migrate_irq ${CPU}
    if [ $? != 0 ]; then
        return 1
    fi
}

# Start up a process that writes to disk; keep track of its PID
bash $LHCS_PATH/tools/do_disk_write_loop > /dev/null 2>&1 &
WRL_ID=$!

until [ $loop = 0 ]
do
  echo "Starting loop '$loop'"
  IRQ_START=`cat /proc/interrupts`

  # Attempt to offline all CPUs
  for cpu in $( get_all_cpus ); do
      if [ "$cpu" = "cpu0" ]; then
          continue
      fi
      do_offline $cpu
      err=$?
      if [ $err != 0 ]; then
          echo "offlining $cpu:  ERROR $err"
      else
          echo "offlining $cpu:  OK"
      fi
      sleep $TM_OFFLINE
  done

#  IRQ_MID=`cat /proc/interrupts`

  # Attempt to online all CPUs
  for cpu in $( get_all_cpus ); do
      if [ "$cpu" = "cpu0" ]; then
          continue
      fi
      do_online $cpu
      err=$?
      if [ $err != 0 ]; then
          echo "onlining $cpu:  ERROR $err"
      else
          echo "onlining $cpu:  OK"
      fi
      sleep $TM_ONLINE
  done

  IRQ_END=`cat /proc/interrupts`

  # Print out a report showing the changes in IRQs
  echo
  echo
  $LHCS_PATH/tools/report_proc_interrupts "$IRQ_START" "$IRQ_END"
  echo

  sleep $TM_DLY
  echo "$CASE:   PASS: Loops left $loop"
  let "loop = loop - 1"

done

exit_clean
