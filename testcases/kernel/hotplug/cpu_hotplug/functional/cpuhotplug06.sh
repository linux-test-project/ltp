#!/bin/sh
#
# Test Case 6 - top
#

export TCID="cpuhotplug06"
export TST_TOTAL=1

# Includes:
. test.sh
. cpuhotplug_testsuite.sh
. cpuhotplug_hotplug.sh

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   Does top work properly when CPU hotplug events occur?

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

do_clean()
{
	pid_is_valid ${TOP_PID} && kill_pid ${TOP_PID}
}

while getopts c:l: OPTION; do
	case $OPTION in
	c)
		CPU_TO_TEST=$OPTARG;;
	l)
		HOTPLUG06_LOOPS=$OPTARG;;
	?)
		usage;;
	esac
done

LOOP_COUNT=1

if tst_virt_hyperv; then
	tst_brkm TCONF "Microsoft Hyper-V detected, no support for CPU hotplug"
fi

if top -v | grep -q htop; then
	tst_brkm TCONF "htop is used instead of top (workaround: alias top='/path/to/real/top')"
fi

if [ $(get_present_cpus_num) -lt 2 ]; then
	tst_brkm TCONF "system doesn't have required CPU hotplug support"
fi

if [ -z "$CPU_TO_TEST" ]; then
	tst_brkm TBROK "Usage: ${0##*/} <CPU to offline>"
fi

# Verify that the specified CPU is available
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
	tst_brkm TCONF "cpu${CPU_TO_TEST} doesn't support hotplug"
fi

# Check that the specified CPU is online; if not, online it
if ! cpu_is_online "${CPU_TO_TEST}" ; then
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
	fi
fi

TST_CLEANUP=do_clean

until [ $LOOP_COUNT -gt $HOTPLUG06_LOOPS ]; do
	# Start up top and give it a little time to run
	top -b -d 00.10 > /dev/null 2>&1 &
	TOP_PID=$!
	sleep 1

	# Now offline the CPU
	if ! offline_cpu ${CPU_TO_TEST} ; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be offlined"
	fi

	# Wait a little time for top to notice the CPU is gone
	sleep 1

	# Check that top hasn't crashed
	if ! pid_is_valid ${TOP_PID} ; then
		tst_resm TFAIL "PID ${TOP_PID} no longer running"
		tst_exit
	fi

	if ! online_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
	fi

	kill_pid ${TOP_PID}

	LOOP_COUNT=$((LOOP_COUNT+1))

done

tst_resm TPASS "PID ${TOP_PID} still running."

tst_exit
