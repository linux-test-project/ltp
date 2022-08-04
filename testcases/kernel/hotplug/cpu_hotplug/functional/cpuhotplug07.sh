#!/bin/sh
#
# Test Case 7
#
# Runs continuous offline/online of CPUs along with
# a kernel compilation load.

export TCID="cpuhotplug07"
export TST_TOTAL=1

# Includes:
. test.sh
. cpuhotplug_testsuite.sh
. cpuhotplug_hotplug.sh

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   What happens when hotplugging during a heavy workload?
Issue:  Hotplug bugs have been found during kernel compiles

EOF

usage()
{
	cat << EOF
	usage: $0 -c cpu -l loop -d directory

	OPTIONS
		-c	cpu which is specified for testing
		-l	number of cycle test
		-d	kernel directory where run this test

EOF
	exit 1
}

do_clean()
{
	kill_pid ${KCOMPILE_LOOP_PID}
}

while getopts c:l:d: OPTION; do
	case $OPTION in
	c)
		CPU_TO_TEST=$OPTARG;;
	l)
		HOTPLUG07_LOOPS=$OPTARG;;
	d)
		KERNEL_DIR=$OPTARG;;
	?)
		usage;;
	esac
done

LOOP_COUNT=1

if tst_virt_hyperv; then
	tst_brkm TCONF "Microsoft Hyper-V detected, no support for CPU hotplug"
fi

if [ $(get_present_cpus_num) -lt 2 ]; then
	tst_brkm TCONF "system doesn't have required CPU hotplug support"
fi

if [ ! -d "${KERNEL_DIR}" ]; then
	tst_brkm TCONF "kernel directory - $KERNEL_DIR - does not exist"
fi

if [ -z "${CPU_TO_TEST}" ]; then
	tst_brkm TBROK "usage: ${0##*/} <CPU to offline> <Kernel \
		source code directory>"
fi

# Validate the specified CPU is available
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
	tst_brkm TCONF "cpu${CPU_TO_TEST} doesn't support hotplug"
fi

if ! cpu_is_online ${CPU_TO_TEST}; then
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
	fi
fi

TST_CLEANUP=do_clean

cpuhotplug_do_kcompile_loop $KERNEL_DIR > /dev/null 2>&1 &
KCOMPILE_LOOP_PID=$!

tst_resm TINFO "initial CPU affinity for kernel compile is: \
	$(get_affinity_mask ${KCOMPILE_LOOP_PID})"

sleep 2

until [ $LOOP_COUNT -gt $HOTPLUG07_LOOPS ]; do

	tst_resm TINFO "Starting loop"

	# Move spin_loop.sh to the CPU to offline.
	set_affinity ${KCOMPILE_LOOP_PID} ${CPU_TO_TEST}

	if ! offline_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be offlined"
	fi

	NEW_CPU=`ps --pid=${KCOMPILE_LOOP_PID} -o psr --no-headers`
	if [ -z "${NEW_CPU}" ]; then
		tst_brkm TBROK "PID ${KCOMPILE_LOOP_PID} no longer running"
	fi
	if [ "${CPU_TO_TEST}" = "${NEW_CPU}" ]; then
		tst_resm TFAIL "process did not change from CPU ${NEW_CPU}"
		tst_exit
	fi

	if ! online_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
	fi

	LOOP_COUNT=$((LOOP_COUNT+1))

done

tst_resm TPASS "turned off CPU ${CPU_TO_TEST}, process migrated to \
	CPU ${NEW_CPU}"

sleep 2

tst_exit
