#!/bin/sh
#
# Test Case 7
#
# Runs continuous offline/online of CPUs along with
# a kernel compilation load.

CASE="hotplug07"
HOTPLUG07_LOOPS=${HOTPLUG07_LOOPS:-${LOOPS}}
loop_one=${HOTPLUG07_LOOPS:-1}

# Includes:
LHCS_PATH=${LHCS_PATH:-".."}
source $LHCS_PATH/include/testsuite.fns
source $LHCS_PATH/include/hotplug.fns

echo "Name:   HotPlug Test - Test Case 7"
echo "Date:   `date`"
echo "Desc:   What happens when hotplugging during a heavy workload?"
echo "Issue:  Hotplug bugs have been found during kernel compiles"
echo

CPU_TO_TEST=${1#cpu}
KERNEL_DIR=$2
if [ -z ${CPU_TO_TEST} ]; then
   echo "Usage:  $0 <CPU to offline> <Kernel source code directory>"
   exit_clean -1
fi
if [ -z ${KERNEL_DIR} ]; then
    for d in `ls -d /usr/src/linux*`; do
        if [ ! -d $d ]; then
            continue
        fi
        if [ ! -f $d/Makefile ]; then
            continue
        fi
        KERNEL_DIR=$d
        break
    done
fi

if [ ! -d ${KERNEL_DIR} ]; then
    echo "Error:  Directory '$KERNEL_DIR' does not exist"
    exit_clean 1
fi

do_clean()
{
    kill_pid ${KCOMPILE_LOOP_PID}
}

bash $LHCS_PATH/tools/do_kcompile_loop $KERNEL_DIR > /dev/null 2>&1 &
KCOMPILE_LOOP_PID=$!

echo get_affinity_mask ${KCOMPILE_LOOP_PID}

cpu_is_online ${CPU_TO_TEST}
if [ $? != 0 ]; then
    online_cpu ${CPU_TO_TEST}
    if [ $? != 0 ]; then
        echo "Error:  CPU${CPU_TO_TEST} cannot be onlined"
        exit_clean 1
    fi
fi

sleep 2

until [ $loop_one = 0 ]
do
  echo "Starting loop '$loop_one'"

  # Move spin_loop.sh to the CPU to offline.
  set_affinity ${KCOMPILE_LOOP_PID} ${CPU_TO_TEST}

  offline_cpu ${CPU_TO_TEST}
  RC=$?
  echo "Offlining cpu${CPU_TO_TEST}: Return Code = ${RC}"

  NEW_CPU=`ps --pid=${KCOMPILE_LOOP_PID} -o psr --no-headers`
  if [ -z ${NEW_CPU} ]; then
      echo "FAIL - PID ${KCOMPILE_LOOP_PID} no longer running"
  elif [ ${CPU_TO_TEST} = ${NEW_CPU} ]; then
      echo "FAIL - process did not change from CPU ${NEW_CPU}"
  else
      echo "PASS - turned off CPU ${CPU_TO_TEST}, process migrated to CPU ${NEW_CPU}"
  fi

  online_cpu ${CPU_TO_TEST}
  RC=$?
  echo "Onlining cpu${CPU_TO_TEST}: Return Code = ${RC}"

  let "loop_one = loop_one - 1"

done

sleep 2

exit_clean
