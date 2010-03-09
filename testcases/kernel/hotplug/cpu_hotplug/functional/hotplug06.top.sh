#!/bin/sh
#
# Test Case 6 - top
#

TST_TOTAL=${HOTPLUG06_LOOPS:-${LOOPS}}
export TCID="hotplug06.top"
export TST_COUNT=1
export TST_TOTAL=${HOTPLUG06_LOOPS:-1}

CPU_TO_TEST=$1
if [ -z "$CPU_TO_TEST" ]; then
	echo "Usage: ${0##*/} <CPU to offline>"
	exit_clean 1
fi

# Includes:
LHCS_PATH=${LHCS_PATH:-${LTPROOT:+$LTPROOT/testcases/bin/cpu_hotplug}}
. $LHCS_PATH/include/testsuite.fns
. $LHCS_PATH/include/hotplug.fns

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   Does top work properly when CPU hotplug events occur?

EOF

# Verify that the specified CPU is available
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
	tst_resm TBROK "CPU${CPU_TO_TEST} not found"
	exit_clean 1
fi

# Check that the specified CPU is online; if not, online it
if ! cpu_is_online "${CPU_TO_TEST}" ; then
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_resm TFAIL "CPU${CPU_TO_TEST} cannot be onlined"
		exit_clean 1
	fi
fi

do_clean()
{
	pid_is_valid ${TOP_PID} && kill_pid ${TOP_PID}
	online_cpu ${CPU_TO_TEST}
}

until [ $TST_COUNT -gt $TST_TOTAL ]; do
	# Start up top and give it a little time to run
	top -b -d 00.10 > /dev/null 2>&1 & 
	TOP_PID=$!
	sleep 1

	# Now offline the CPU
	if ! offline_cpu ${CPU_TO_TEST} ; then
		tst_resm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
		exit_clean 1
	fi

	# Wait a little time for top to notice the CPU is gone
	sleep 1

	# Check that top hasn't crashed
	if pid_is_valid ${TOP_PID} ; then
		tst_resm TPASS "PID ${TOP_PID} still running."
		online_cpu ${CPU_TO_TEST}
		kill_pid ${TOP_PID}
	else
		tst_resm TFAIL "PID ${TOP_PID} no longer running"
		exit_clean 1
	fi

	: $(( TST_COUNT += 1 ))

done

exit_clean
