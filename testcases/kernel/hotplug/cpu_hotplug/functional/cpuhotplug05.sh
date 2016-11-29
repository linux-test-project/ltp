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

tst_check_cmds sar

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

until [ $LOOP_COUNT -gt $HOTPLUG05_LOOPS ]; do

	# Start up SAR and give it a couple cycles to run
	sar 1 0 >/dev/null 2>&1 &
	sleep 2
	# "sar 1 0" is supported before 'sysstat-8.1.4(include sar)',
	# after that use "sar 1" instead of. Use 'ps -C sar' to check.
	if ps -C sar >/dev/null 2>&1; then
		pkill sar
		sar -P ALL 1 0 > $TMP/log_$$ &
	else
		sar -P ALL 1 > $TMP/log_$$ &
	fi
	sleep 2
	SAR_PID=$!

	# Verify that SAR has correctly listed the missing CPU
	while ! awk '{print $8}' $TMP/log_$$ | grep -i "^0.00"; do
		tst_brkm TBROK "CPU${CPU_TO_TEST} Not Found on SAR!"
	done
	time=`date +%X`
	sleep .5

	# Verify that at least some of the CPUs are offline
	NUMBER_CPU_OFF=$(grep "$time" $TMP/log_$$ | awk '{print $8}' \
		|grep -i "^0.00" | wc -l)
	if [ ${NUMBER_CPU_OFF} -eq 0 ]; then
		tst_brkm TBROK "no CPUs found offline"
	fi

	# Online the CPU
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be onlined line"
	fi

	sleep 2
	time=$(date +%T)
	sleep .5

	# Check that SAR registered the change in CPU online/offline states
	NEW_NUMBER_CPU_OFF=$(grep "$time" $TMP/log_$$|awk '{print $8}' \
		| grep -i "^0.00"| wc -l)
	NUMBER_CPU_OFF=$((NUMBER_CPU_OFF-1))
	if [ "$NUMBER_CPU_OFF" != "$NEW_NUMBER_CPU_OFF" ]; then
		tst_resm TFAIL "no change in number of offline CPUs was found."
		tst_exit
	fi

	offline_cpu ${CPU_TO_TEST}
	kill_pid ${SAR_PID}

	LOOP_COUNT=$((LOOP_COUNT+1))

done

tst_resm TPASS "CPU was found after turned on."

tst_exit
