#!/bin/sh
#
# Test Case 3
#

export TCID="cpuhotplug03"
export TST_TOTAL=1

# Includes:
. test.sh
. cpuhotplug_testsuite.sh
. cpuhotplug_hotplug.sh

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   Do tasks get scheduled to a newly on-lined CPU?

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

	# Restore CPU states
	set_all_cpu_states "$cpu_states"
}

while getopts c:l: OPTION; do
case $OPTION in
	c)
		CPU_TO_TEST=$OPTARG;;
	l)
		HOTPLUG03_LOOPS=$OPTARG;;
	?)
		usage;;
	esac
done

LOOP_COUNT=1

cpus_num=$(get_present_cpus_num)
if [ $cpus_num -lt 2 ]; then
	tst_brkm TCONF "system doesn't have required CPU hotplug support"
fi

if [ -z $CPU_TO_TEST ]; then
	tst_brkm TBROK "usage: ${0##*} <CPU to online>"
fi

# Validate the specified CPU is available
if ! cpu_is_valid "${CPU_TO_TEST}" ; then
	tst_brkm TCONF "cpu${CPU_TO_TEST} doesn't support hotplug"
fi

TST_CLEANUP=do_clean

cpu_states=$(get_all_cpu_states)

until [ $LOOP_COUNT -gt $HOTPLUG03_LOOPS ]; do

	# Turns on all CPUs
	for i in $( get_hotplug_cpus ); do
            if ! cpu_is_online $i; then
				if ! online_cpu $i; then
                    tst_brkm TBROK "Could not online cpu $i"
                fi
            fi
	done

	if ! offline_cpu ${CPU_TO_TEST} ; then
		tst_resm TBROK "CPU${CPU_TO_TEST} cannot be offlined"
	fi

	# Start up a number of processes equal to twice the number of
	# CPUs we have.  This is to help ensure we've got enough processes
	# that at least one will migrate to the new CPU.  Store the PIDs
	# so we can kill them later.
	number_of_procs=$((cpus_num*2))
	until [ $number_of_procs -eq 0 ]; do
		cpuhotplug_do_spin_loop > /dev/null 2>&1 &
		echo $! >> /var/run/hotplug4_$$.pid
		number_of_procs=$((number_of_procs-1))
	done

	ps aux | head -n 1
	ps aux | grep cpuhotplug_do_spin_loop

	# Online the CPU
	tst_resm TINFO "Onlining CPU ${CPU_TO_TEST}"
	if ! online_cpu ${CPU_TO_TEST}; then
		tst_brkm TBROK "CPU${CPU_TO_TEST} cannot be onlined"
	fi

	sleep 1

	# Verify at least one process has migrated to the new CPU
	# Since procps v3.3.15, we need to accurately select command name
	# by -C option, because procps cannot trucate normal command name
	# to 15 characters by default).
	ps -o psr -o command --no-headers -C cpuhotplug_do_s
	if [ $? -ne 0 ]; then
		tst_brkm TBROK "No cpuhotplug_do_spin_loop processes \
			found on any processor"
	fi
	NUM=`ps -o psr -o command --no-headers -C cpuhotplug_do_s \
		| sed -e "s/^ *//" | cut -d' ' -f 1 | grep "^${CPU_TO_TEST}$" \
		| wc -l`
	if [ $NUM -lt 1 ]; then
		tst_resm TFAIL "No cpuhotplug_do_spin_loop processes found on \
			CPU${CPU_TO_TEST}"
		tst_exit
	fi

	do_clean

	LOOP_COUNT=$((LOOP_COUNT+1))
done

tst_resm TPASS "$NUM cpuhotplug_do_spin_loop processes found on \
	CPU${CPU_TO_TEST}"

tst_exit
