#!/bin/sh
#
# Test Case 5 - sar
#

export TCID="cpuhotplug05"
export TST_TOTAL=1
export LC_TIME="POSIX"

# Includes:
. test.sh
. cpuhotplug_testsuite.sh
. cpuhotplug_hotplug.sh

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   Does sar behave properly during CPU hotplug events?

EOF

usage()
{
	cat << EOF
	usage: $0 -c cpu -l loop -d directory

	OPTIONS
		-c  cpu which is specified for testing
		-l  number of cycle test
		-d  directory used to lay file

EOF
	exit 1
}

do_clean()
{
	pid_is_valid ${SAR_PID} && kill_pid ${SAR_PID}
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
	fi
}

get_field()
{
	echo "$1" | awk "{print \$$2}"
}

while getopts c:l:d: OPTION; do
	case $OPTION in
	c)
		CPU_TO_TEST=$OPTARG;;
	l)
		HOTPLUG05_LOOPS=$OPTARG;;
	d)
		TMP=$OPTARG;;
	?)
		usage;;
	esac
done

LOOP_COUNT=1

tst_require_cmds sar

if tst_virt_hyperv; then
	tst_brkm TCONF "Microsoft Hyper-V detected, no support for CPU hotplug"
fi

if [ $(get_present_cpus_num) -lt 2 ]; then
	tst_brkm TCONF "system doesn't have required CPU hotplug support"
fi

if [ -z "$CPU_TO_TEST" ]; then
	tst_brkm TBROK "usage: ${0##*} <CPU to offline>"
fi

# Validate the specified CPU is available
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
	tst_brkm TCONF "cpu${CPU_TO_TEST} doesn't support hotplug"
fi

# Check that the specified CPU is offline; if not, offline it
if cpu_is_online "${CPU_TO_TEST}" ; then
	if ! offline_cpu ${CPU_TO_TEST} ; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be offlined"
	fi
fi

TST_CLEANUP=do_clean

LOG_FILE="$TMP/log_$$"

until [ $LOOP_COUNT -gt $HOTPLUG05_LOOPS ]; do

	# Start up SAR and give it a couple cycles to run
	sar 1 0 >/dev/null 2>&1 &
	sleep 2
	# "sar 1 0" is supported before 'sysstat-8.1.4(include sar)',
	# after that use "sar 1" instead of. Use 'ps -C sar' to check.
	if ps -C sar >/dev/null 2>&1; then
		pkill sar
		sar -P "$CPU_TO_TEST" 1 0 > "$LOG_FILE" &
	else
		sar -P "$CPU_TO_TEST" 1 > "$LOG_FILE" &
	fi
	sleep 2
	SAR_PID=$!

	# Since the CPU is offline, SAR should display all the 6 fields
	# of CPU statistics as '0.00'
	offline_status=$(tail -n 1 "$LOG_FILE")
	if [ -z "$offline_status" ]; then
		tst_brkm TBROK "SAR output file is empty"
	fi

	cpu_field=$(get_field "$offline_status" "2")
	if [ "${cpu_field}" = "CPU" ]; then
		# Since sysstat-11.7.1, sar/sadf didn't display offline CPU
		tst_resm TINFO "SAR didn't display offline CPU"
	else
		for i in $(seq 3 8); do
			field=$(get_field "$offline_status" "$i")
			if [ "$field" != "0.00" ]; then
				tst_brkm TBROK "Field $i is '$field', '0.00' expected"
			fi
		done
	fi

	# Online the CPU
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
	fi

	sleep 2

	# Check that SAR registered the change in CPU online/offline states
	online_status=$(tail -n 1 "$LOG_FILE")
	check_passed=0
	for i in $(seq 3 8); do
		field_online=$(get_field "$online_status" "$i")

		if [ "$field_online" != "0.00" ]; then
			check_passed=1
			break
		fi
	done

	if [ $check_passed -eq 0 ]; then
		tst_resm TFAIL "No change in the CPU statistics"
		tst_exit
	fi

	if ! offline_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be offlined"
	fi

	kill_pid ${SAR_PID}

	LOOP_COUNT=$((LOOP_COUNT+1))

done

tst_resm TPASS "SAR updated statistics after the CPU was turned on."

tst_exit
