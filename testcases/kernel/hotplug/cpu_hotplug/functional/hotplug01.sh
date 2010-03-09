#!/bin/sh
#
# Test Case 1
#
# Based on script by Ashok Raj <ashok.raj@intel.com>
# Modified by Mark D and Bryce, Aug '05.

HOTPLUG01_LOOPS=${HOTPLUG01_LOOPS:-${LOOPS}}
export TCID="hotplug01"
export TST_COUNT=1
export TST_TOTAL=${HOTPLUG01_LOOPS:-1}

CPU_TO_TEST=${1#cpu}
if [ -z "${CPU_TO_TEST}" ]; then
	echo "usage: ${0##*/} <CPU to online>"
	exit 1 
fi

# Includes:
LHCS_PATH=${LHCS_PATH:-$LTPROOT/testcases/bin/cpu_hotplug}
. $LHCS_PATH/include/testsuite.fns
. $LHCS_PATH/include/hotplug.fns

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   What happens to disk controller interrupts when offlining CPUs?

EOF

# Time delay after an online of cpu
TM_ONLINE=${HOTPLUG01_TM_ONLINE:-1} 

# Time delay after offline of cpu
TM_OFFLINE=${HOTPLUG01_TM_OFFLINE:-1} 

# Time delay before start of entire new cycle.
TM_DLY=${HOTPLUG01_TM_DLY:-6}	

if ! type -P perl > /dev/null; then
	tst_brk TCONF "analysis script - report_proc_interrupts - requires perl"
	exit 1
fi

if ! get_all_cpus >/dev/null 2>&1; then
	tst_brkm TCONF "system doesn't have required CPU hotplug support"
	exit 1
fi

# Validate the specified CPU exists
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
	tst_resm TFAIL "cpu${CPU_TO_TEST} not found"
	exit 1
fi

CPU_COUNT=0
cpustate=1

if ! cpu_is_online "${CPU_TO_TEST}" ; then
	if ! online_cpu ${CPU_TO_TEST} ; then
		tst_resm TFAIL "Could not online cpu $CPU_TO_TEST"
		exit_clean 1
	fi
	cpustate=0
fi

# do_clean()
#
#  Callback to be executed when script exits from a user interrupt
#  or regular program termination
#
do_clean()
{
	kill_pid ${WRL_ID}

	# Turns off the cpus that were off before the test start
	tst_resm TINFO "Return to previous state. CPU count = ${CPU_COUNT}"
	until [ $CPU_COUNT -eq 0 ]; do
		offline_cpu=$(eval "echo \$OFFLINE_CPU_${CPU_COUNT}")
		tst_resm TINFO "CPU = $CPU_COUNT @on = $offline_cpu"
		offline_cpu $offline_cpu
		: $(( CPU_COUNT -= 1 ))
	done			
	if [ "x${cpustate}" = x1 ]; then
		online_cpu ${CPU_TO_TEST}
	else
		offline_cpu ${CPU_TO_TEST}
	fi
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
	migrate_irq ${CPU} ${IRQS}
	offline_cpu ${CPU}
	if [ $? -ne 0 ]; then
		if [ "$CPU" -ne 0 ]; then
			: $(( CPU_COUNT += 1 ))
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

# Start up a process that writes to disk; keep track of its PID
$LHCS_PATH/tools/do_disk_write_loop > /dev/null 2>&1 &
WRL_ID=$!

RC=0
until [ $TST_COUNT -gt $TST_TOTAL -o $RC -ne 0 ]
do

	tst_resm TINFO "Starting loop"
	IRQ_START=$(cat /proc/interrupts)

	# Attempt to offline all CPUs
	for cpu in $( get_all_cpus ); do
		if [ "$cpu" = "cpu0" ]; then
			continue
		fi
		do_offline $cpu
		err=$?
		if [ $err -ne 0 ]; then
			tst_resm TBROK "offlining $cpu failed: $err"
			RC=2
		else
			tst_resm TINFO "offlining $cpu was ok"
		fi
		sleep $TM_OFFLINE
	done

	#IRQ_MID=`cat /proc/interrupts`

	# Attempt to online all CPUs
	for cpu in $( get_all_cpus ); do
		if [ "$cpu" = "cpu0" ]; then
			continue
		fi
		do_online $cpu
		err=$?
		if [ $err -ne 0 ]; then
			tst_resm TBROK "onlining $cpu failed: $err"
			RC=2
		else
			tst_resm TINFO "onlining $cpu was ok"
		fi
		sleep $TM_ONLINE
	done

	IRQ_END=`cat /proc/interrupts`

	# Print out a report showing the changes in IRQs
	echo
	echo
	$LHCS_PATH/tools/report_proc_interrupts "$IRQ_START" "$IRQ_END"
	echo

	if [ $RC -eq 0 ] ; then

		sleep $TM_DLY
		: $(( TST_COUNT += 1 ))

	fi

done

exit_clean $RC
