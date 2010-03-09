#!/bin/sh
#
# Test Case 3
#

HOTPLUG03_LOOPS=${HOTPLUG03_LOOPS:-${LOOPS}}
export TST_COUNT=1
export TST_TOTAL=${HOTPLUG03_LOOPS:-1}
export TCID="hotplug03"

CPU_TO_TEST=${1#cpu}
if [ -z $CPU_TO_TEST ]; then
	echo "usage: ${0##*} <CPU to online>"
	exit 1
fi

# Includes:
LHCS_PATH=${LHCS_PATH:-${LTPROOT:+$LTPROOT/testcases/bin/cpu_hotplug}}
. $LHCS_PATH/include/testsuite.fns
. $LHCS_PATH/include/hotplug.fns

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   Do tasks get scheduled to a newly on-lined CPU?

EOF

# Verify the specified CPU exists
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
	tst_resm TCONF "CPU${CPU_TO_TEST} not found"
	exit_clean 1
fi

# do_clean()
#
#  Callback to be executed when script exits from a user interrupt
#  or regular program termination
#
do_clean()
{
	# Kill all the processes we started up and get rid of their pid files
	if [ -e "/var/run/hotplug4_$$.pid" ]; then
		for i in `cat /var/run/hotplug4_$$.pid`; do
			kill_pid $i
		done
		rm /var/run/hotplug4_$$.pid
	fi

	# Turn off the CPUs that were off before the test start
	until [ $cpu -eq 0 ];do
		offline_cpu $(eval "echo \$on_${cpu}")
		: $(( cpu -= 1 ))
	done
}

until [ $TST_COUNT -gt $TST_TOTAL ]; do 
	cpu=0
	number_of_cpus=0

	# Turns on all CPUs and saves their states
	for i in $( get_all_cpus ); do 
		if ! online_cpu $1; then
			: $(( cpu += 1 ))
			eval "on_${cpu}=$i"
		fi
		: $(( number_of_cpus += 1 ))
	done

	if ! offline_cpu ${CPU_TO_TEST} ; then
		tst_resm TBAIL "CPU${CPU_TO_TEST} cannot be offlined"
		exit_clean 1
	fi

	# Start up a number of processes equal to twice the number of
	# CPUs we have.  This is to help ensure we've got enough processes
	# that at least one will migrate to the new CPU.  Store the PIDs 
	# so we can kill them later.
	: $(( number_of_cpus *= 2 ))
	until [ $number_of_cpus -eq 0 ]; do
		$LHCS_PATH/tools/do_spin_loop > /dev/null 2>&1 &
		echo $! >> /var/run/hotplug4_$$.pid
		: $(( number_of_cpus -= 1 ))
	done

	ps aux | head -n 1
	ps aux | grep do_spin_loop

	# Online the CPU
	tst_resm TINFO "Onlining CPU ${CPU_TO_TEST}"
	online_cpu ${CPU_TO_TEST}
	RC=$?
	if [ $RC -eq 0 ]; then
		tst_resm TFAIL "CPU${CPU_TO_TEST} cannot be onlined"
		exit_clean 1
	fi

	sleep 1

	# Verify at least one process has migrated to the new CPU
	ps -o psr -o command --no-headers -C do_spin_loop
	RC=$?
	NUM=`ps -o psr -o command --no-headers -C do_spin_loop | sed -e "s/^ *//" | cut -d' ' -f 1 | grep "^${CPU_TO_TEST}$" | wc -l`
	if [ $RC -eq 0 ]; then
		tst_resm TBROK "No do_spin_loop processes found on any processor"
	elif [ $NUM -lt 1 ]; then
		tst_resm TFAIL "No do_spin_loop processes found on CPU${CPU_TO_TEST}"
	else
		tst_resm TPASS "$NUM do_spin_loop processes found on CPU${CPU_TO_TEST}"
	fi

	do_clean

	: $(( TST_COUNT +=1 ))
done

exit_clean
