#!/bin/sh
#
# Test Case 7
#
# Runs continuous offline/online of CPUs along with
# a kernel compilation load.

TST_TOTAL=${HOTPLUG07_LOOPS:-${LOOPS}}
export TCID="hotplug07"
export TST_COUNT=1
export TST_TOTAL=${HOTPLUG07_LOOPS:-1}

# Includes:
LHCS_PATH=${LHCS_PATH:-${LTPROOT:+$LTPROOT/testcases/bin/cpu_hotplug}}
. $LHCS_PATH/include/testsuite.fns
. $LHCS_PATH/include/hotplug.fns

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   What happens when hotplugging during a heavy workload?
Issue:  Hotplug bugs have been found during kernel compiles

EOF

CPU_TO_TEST=${1#cpu}
KERNEL_DIR=${2:-/usr/src/linux}
if [ -z "${CPU_TO_TEST}" ]; then
	echo "usage: ${0##*/} <CPU to offline> <Kernel source code directory>"
	exit_clean 1
fi
if [ ! -d "${KERNEL_DIR}" ]; then
	tst_resm TCONF "kernel directory - $KERNEL_DIR - does not exist"
	exit_clean 1
fi

do_clean()
{
	kill_pid ${KCOMPILE_LOOP_PID}
}

$LHCS_PATH/tools/do_kcompile_loop $KERNEL_DIR > /dev/null 2>&1 &
KCOMPILE_LOOP_PID=$!

tst_resm TINFO "initial CPU affinity for kernel compile is: $(get_affinity_mask ${KCOMPILE_LOOP_PID})"

if ! cpu_is_online ${CPU_TO_TEST}; then
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_resm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
		exit_clean 1
	fi
fi

sleep 2

until [ $TST_COUNT -gt $TST_TOTAL ]; do

	tst_resm TINFO "Starting loop"

	# Move spin_loop.sh to the CPU to offline.
	set_affinity ${KCOMPILE_LOOP_PID} ${CPU_TO_TEST}

	offline_cpu ${CPU_TO_TEST}
	RC=$?
	echo "Offlining cpu${CPU_TO_TEST}: Return Code = ${RC}"

	NEW_CPU=`ps --pid=${KCOMPILE_LOOP_PID} -o psr --no-headers`
	if [ -z "${NEW_CPU}" ]; then
		tst_resm TBROK "PID ${KCOMPILE_LOOP_PID} no longer running"
		exit_clean 1
	elif [ "${CPU_TO_TEST}" = "${NEW_CPU}" ]; then
		tst_resm TFAIL "process did not change from CPU ${NEW_CPU}"
	else
		tst_resm TPASS "turned off CPU ${CPU_TO_TEST}, process migrated to CPU ${NEW_CPU}"
	fi

	online_cpu ${CPU_TO_TEST}
	RC=$?

	tst_resm TINFO "Onlining cpu${CPU_TO_TEST}: Return Code = ${RC}"

	: $(( TST_COUNT += 1 ))

done

sleep 2

exit_clean
