#!/bin/sh
#
# Test Case 2
#

export TCID="cpuhotplug02"
export TST_TOTAL=1

# Includes:
. test.sh
. cpuhotplug_testsuite.sh
. cpuhotplug_hotplug.sh

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   What happens to a process when its CPU is offlined?

EOF

usage()
{
	cat << EOF
	usage: $0 -c cpu -l loop

	OPTIONS
		-c  cpu which is specified for testing
		-l  number of cycle test

EOF
	exit 1
}

# do_clean()
#
#  Callback to be executed when script exits from a user interrupt
#  or regular program termination.
#
do_clean()
{
	kill_pid ${SPIN_LOOP_PID}
}

while getopts c:l: OPTION; do
	case $OPTION in
	c)
		CPU_TO_TEST=$OPTARG;;
	l)
		HOTPLUG02_LOOPS=$OPTARG;;
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

if [ -z "${CPU_TO_TEST}" ]; then
	tst_brkm TBROK "usage: ${0##*/} <CPU to online>"
fi

# Validate the specified CPU is available
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
	tst_brkm TCONF "cpu${CPU_TO_TEST} doesn't support hotplug"
fi

# Validate the specified CPU is online; if not, online it
if ! cpu_is_online "${CPU_TO_TEST}" ; then
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
	fi
fi

TST_CLEANUP=do_clean

# Start up a process that just uses CPU cycles
cpuhotplug_do_spin_loop > /dev/null&
SPIN_LOOP_PID=$!

sleep 5
until [ $LOOP_COUNT -gt $HOTPLUG02_LOOPS ]; do
	# Move spin_loop.sh to the CPU to offline.
	set_affinity ${SPIN_LOOP_PID} ${CPU_TO_TEST}

	# Verify the process migrated to the CPU we intended it to go to
	if ! offline_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be offlined"
	fi

	NEW_CPU=`ps --pid=${SPIN_LOOP_PID} -o psr --no-headers`
	if [ -z "${NEW_CPU}" ]; then
		tst_brkm TBROK "PID ${SPIN_LOOP_PID} no longer running"
	fi
	if [ ${CPU_TO_TEST} = ${NEW_CPU} ]; then
		tst_resm TFAIL "process did not change from CPU ${NEW_CPU}"
		tst_exit
	fi

	# Turn the CPU back online just to see what happens.
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
	fi

	LOOP_COUNT=$((LOOP_COUNT+1))
done

tst_resm TPASS "turned off CPU ${CPU_TO_TEST}, process migrated to \
	CPU ${NEW_CPU}"

sleep 2

tst_exit
