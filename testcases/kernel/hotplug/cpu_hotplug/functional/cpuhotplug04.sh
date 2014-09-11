#!/bin/sh
#
# Test Case 4
#

export TCID="cpuhotplug04"
export TST_TOTAL=1

# Includes:
. test.sh
. cpuhotplug_testsuite.sh
. cpuhotplug_hotplug.sh

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   Does it prevent us from offlining the last CPU?

EOF

usage()
{
	cat << EOF
	usage: $0 -l loop

	OPTIONS
		-l  number of cycle test

EOF
	exit 1
}

do_clean()
{
	# Online the ones that were on initially
	until [ $cpu -eq 0 ]; do
		online_cpu $(eval "echo \$on_${cpu}")
		cpu=$((cpu-1))
	done

	# Return CPU 0 to its initial state
	if [ $cpustate = 1 ]; then
		online_cpu 0
	else
		offline_cpu 0
	fi
}

while getopts l: OPTION; do
	case $OPTION in
	l)
		HOTPLUG04_LOOPS=$OPTARG;;
	?)
		usage;;
	esac
done

LOOP_COUNT=1

get_cpus_num
if [ $? -lt 2 ]; then
	tst_brkm TCONF "system doesn't have required CPU hotplug support"
fi

TST_CLEANUP=do_clean

until [ $LOOP_COUNT -gt $HOTPLUG04_LOOPS ]; do
	cpu=0
	cpustate=1

	# Online all the CPUs' keep track of which were already on
	for i in $(get_all_cpus); do
		if [ "$i" != "cpu0" ]; then
			if ! cpu_is_online $i; then
				if ! online_cpu $i; then
					tst_brkm TBROK "$i cannot be onlined"
				fi
			fi
			cpu=$((cpu+1))
			eval "on_${cpu}=$i"
			echo $i
		else
			if online_cpu $i; then
				cpustate=0
			fi
		fi
	done

	# Now offline all the CPUs
	for i in $(get_all_cpus); do
		if ! offline_cpu $i; then
			if [ "x$i" != "xcpu0" ]; then
				tst_resm TFAIL "Did not offline first CPU (offlined $i instead)"
				tst_exit
			fi
		fi
	done

	LOOP_COUNT=$((LOOP_COUNT+1))

done

tst_resm TPASS "Successfully offlined first CPU, $i"

tst_exit
