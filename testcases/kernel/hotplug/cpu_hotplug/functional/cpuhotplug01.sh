#!/bin/sh
#
# Test Case 1
#
# Based on script by Ashok Raj <ashok.raj@intel.com>
# Modified by Mark D and Bryce, Aug '05.

export TCID="cpuhotplug01"
export TST_TOTAL=1

# Includes:
. test.sh
. cpuhotplug_testsuite.sh
. cpuhotplug_hotplug.sh

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   What happens to disk controller interrupts when offlining CPUs?

EOF

usage()
{
	cat << EOF
	usage: $0 -c cpu -l loop -n timeon -f timeoff -e timed

	OPTIONS
		-c  cpu which is specified for testing
		-l  number of cycle test
		-n	time delay after an online of cpu
		-f	time delay after offline of cpu
		-e	time delay before start of entire new cycle

EOF
	exit 1
}

# do_clean()
#
#  Callback to be executed when script exits from a user interrupt
#  or regular program termination
#
do_clean()
{
	kill_pid ${WRL_ID}

	# Restore CPU states
	set_all_cpu_states "$cpu_states"
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
	migrate_irq "${CPU}" "${IRQS}"
	offline_cpu ${CPU}
	if [ $? -ne 0 ]; then
		if [ "$CPU" -ne 0 ]; then
			CPU_COUNT=$((CPU_COUNT+1))
			eval "OFFLINE_CPU_${CPU_COUNT}=$1"
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
	if [ $? -ne 0 ]; then
		return 1
	fi
	migrate_irq ${CPU}
	if [ $? -ne 0 ]; then
		return 1
	fi
}

while getopts c:l:n:f:e: OPTION; do
	case $OPTION in
	c)
		CPU_TO_TEST=$OPTARG;;
	l)
		HOTPLUG01_LOOPS=$OPTARG;;
	n)
		TM_ONLINE=$OPTARG;;
	f)
		TM_OFFLINE=$OPTARG;;
	e)
		TM_DLY=$OPTARG;;
	?)
		usage;;
	esac
done

LOOP_COUNT=1

tst_require_cmds perl

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

if ! cpu_is_online "${CPU_TO_TEST}" ; then
	if ! online_cpu ${CPU_TO_TEST} ; then
		tst_brkm TBROK "Could not online cpu $CPU_TO_TEST"
	fi
fi

TST_CLEANUP=do_clean

cpu_states=$(get_all_cpu_states)

CPU_COUNT=0

# Start up a process that writes to disk; keep track of its PID
cpuhotplug_do_disk_write_loop > /dev/null 2>&1 &
WRL_ID=$!

until [ $LOOP_COUNT -gt $HOTPLUG01_LOOPS ]
do

	tst_resm TINFO "Starting loop"
	IRQ_START=$(cat /proc/interrupts)

	# Attempt to offline all CPUs
	for cpu in $( get_hotplug_cpus ); do
		if [ "$cpu" = "cpu0" ]; then
			continue
		fi
		do_offline $cpu
		err=$?
		if [ $err -ne 0 ]; then
			tst_brkm TBROK "offlining $cpu failed: $err"
		else
			tst_resm TINFO "offlining $cpu was ok"
		fi
		sleep $TM_OFFLINE
	done

	# Attempt to online all CPUs
	for cpu in $( get_hotplug_cpus ); do
		if [ "$cpu" = "cpu0" ]; then
			continue
		fi
		do_online $cpu
		err=$?
		if [ $err -ne 0 ]; then
			tst_brkm TBROK "onlining $cpu failed: $err"
		else
			tst_resm TINFO "onlining $cpu was ok"
		fi
		sleep $TM_ONLINE
	done

	IRQ_END=`cat /proc/interrupts`

	# Print out a report showing the changes in IRQs
	echo
	echo
	cpuhotplug_report_proc_interrupts "$IRQ_START" "$IRQ_END"
	echo

	sleep $TM_DLY
	LOOP_COUNT=$((LOOP_COUNT+1))

done

tst_resm TPASS "online and offline cpu${CPU} when writing disk"

tst_exit
