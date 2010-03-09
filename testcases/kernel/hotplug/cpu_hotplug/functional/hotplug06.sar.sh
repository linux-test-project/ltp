#!/bin/sh
#
# Test Case 6 - sar
#

TST_TOTAL=${HOTPLUG06_LOOPS:-${LOOPS}}
export TCID="hotplug06.sar"
export TMP=${TMP:=/tmp}
export TST_COUNT=1
export TST_TOTAL=${HOTPLUG06_LOOPS:-1}

CPU_TO_TEST=$1
if [ -z "$CPU_TO_TEST" ]; then
	echo "usage: ${0##*} <CPU to offline>"
	exit 1
fi

# Includes:
LHCS_PATH=${LHCS_PATH:-${LTPROOT:+$LTPROOT/testcases/bin/cpu_hotplug}}
. $LHCS_PATH/include/testsuite.fns
. $LHCS_PATH/include/hotplug.fns

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   Does sar behave properly during CPU hotplug events?

EOF

# Verify the specified CPU is available
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
	tst_resm TBROK"CPU${CPU_TO_TEST} not found"
	exit_clean 1
fi

# Check that the specified CPU is offline; if not, offline it
if cpu_is_online "${CPU_TO_TEST}" ; then
	if ! offline_cpu ${CPU_TO_TEST} ; then
		tst_resm TBROK "CPU${CPU_TO_TEST} cannot be offlined"
		exit_clean 1
	fi
fi

do_clean()
{
	kill_pid ${SAR_PID}
}

until [ $TST_COUNT -gt $TST_TOTAL ]; do
	# Start up SAR and give it a couple cycles to run
	sar -P ALL 1 0 > $TMP/log_$$ &
	sleep 2
	SAR_PID=$!
	
	# Verify that SAR has correctly listed the missing CPU as 'nan'
	while ! grep -iq nan $TMP/log_$$; do
		tst_resm TFAIL "CPU${CPU_TO_TEST} Not Found on SAR!"
		exit_clean 1
	done
	time=`date +%X`
	sleep .5

	# Verify that at least some of the CPUs are offline
	NUMBER_CPU_OFF=$(grep "$time" $TMP/log_$$ | grep -i nan | wc -l)
	if [ ${NUMBER_CPU_OFF} -eq 0 ]; then
		tst_resm TBROK "no CPUs found offline"
		exit_clean 1
	fi

	# Online the CPU
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_resm TFAIL "CPU${CPU_TO_TEST} cannot be onlined line"
		exit_clean 1
	fi
	
	sleep 1
	time=$(date +%T)
	sleep .5

	# Check that SAR registered the change in CPU online/offline states
	NEW_NUMBER_CPU_OFF=$(grep "$time" $TMP/log_$$ | grep -i nan | wc -l)
	: $(( NUMBER_CPU_OFF -= 1 ))
	if [ "$NUMBER_CPU_OFF" = "$NEW_NUMBER_CPU_OFF" ]; then
		tst_resm TPASS "CPU was found after turned on."
	else
		tst_resm TFAIL "no change in number of offline CPUs was found."
	fi

	: $(( TST_COUNT += 1 ))

done

exit_clean
