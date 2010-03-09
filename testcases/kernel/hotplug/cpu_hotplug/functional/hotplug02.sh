#!/bin/sh
#
# Test Case 2
#

HOTPLUG02_LOOPS=${HOTPLUG02_LOOPS:-${LOOPS}}
export TCID="hotplug02"
export TST_COUNT=1
export TST_TOTAL=${HOTPLUG02_LOOPS:-1}

CPU_TO_TEST=$1
if [ -z "$CPU_TO_TEST" ]; then
	echo "usage: ${0##*} <CPU to online>"
	exit 1
fi

# Includes:
LHCS_PATH=${LHCS_PATH:-$LTPROOT/testcases/bin/cpu_hotplug}
. $LHCS_PATH/include/testsuite.fns
. $LHCS_PATH/include/hotplug.fns

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   What happens to a process when its CPU is offlined?

EOF

# Start up a process that just uses CPU cycles
$LHCS_PATH/tools/do_spin_loop > /dev/null&
SPIN_LOOP_PID=$!

# Validate the specified CPU exists
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
	tst_resm TBROK "cpu${CPU_TO_TEST} not found"
	exit_clean 1
fi

# do_clean()
#
#  Callback to be executed when script exits from a user interrupt
#  or regular program termination.
#
do_clean()
{
	kill_pid ${SPIN_LOOP_PID}
}

# Validate the specified CPU is online; if not, online it
if ! cpu_is_online "${CPU_TO_TEST}" ; then
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_resm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
		exit_clean 1
	fi
fi

sleep 5
until [ $TST_COUNT -gt $TST_TOTAL ]; do
	# Move spin_loop.sh to the CPU to offline.
	set_affinity ${SPIN_LOOP_PID} ${CPU_TO_TEST}

	# Verify the process migrated to the CPU we intended it to go to
	offline_cpu ${CPU_TO_TEST}
	NEW_CPU=`ps --pid=${SPIN_LOOP_PID} -o psr --no-headers`
	if [ -z "${NEW_CPU}" ]; then
		tst_resm TBROK "PID ${SPIN_LOOP_PID} no longer running"
		exit_clean 1
	elif [ ${CPU_TO_TEST} = ${NEW_CPU} ]; then
		tst_resm TFAIL "process did not change from CPU ${NEW_CPU}"
		exit_clean 1
	fi
	tst_resm TPASS "turned off CPU ${CPU_TO_TEST}, process migrated to CPU ${NEW_CPU}"
	
	# Turn the CPU back online just to see what happens.
	online_cpu ${CPU_TO_TEST}
	: $(( TST_COUNT += 1 ))
done

sleep 2

exit_clean
