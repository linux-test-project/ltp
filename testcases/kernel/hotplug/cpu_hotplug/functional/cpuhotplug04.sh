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
	# Restore CPU states
	set_all_cpu_states "$cpu_states"
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

cpus_num=$(get_present_cpus_num)
if [ $cpus_num -lt 2 ]; then
	tst_brkm TCONF "system doesn't have required CPU hotplug support"
fi

if [ $(get_hotplug_cpus_num) -lt 1 ]; then
	tst_brkm TCONF "system doesn't have at least one hotpluggable CPU"
fi

TST_CLEANUP=do_clean

cpu_states=$(get_all_cpu_states)

until [ $LOOP_COUNT -gt $HOTPLUG04_LOOPS ]; do

	# Online all the hotpluggable CPUs
	for i in $(get_hotplug_cpus); do
		if ! cpu_is_online $i; then
			if ! online_cpu $i; then
				tst_brkm TBROK "$i can not be onlined"
			fi
		fi
	done

	# Now offline them
	cpu=0
	for i in $(get_hotplug_cpus); do
		cpu=$((cpu + 1))

		# If all the CPUs are hotpluggable, we expect
		# that the kernel will refuse to offline the last CPU.
		# If only some of the CPUs are hotpluggable,
		# they all can be offlined.
		if [ $cpu -eq $cpus_num ]; then
			if offline_cpu $i 2> /dev/null; then
				tst_brkm TFAIL "Have we just offlined the last CPU?"
			else
				tst_resm TPASS "System prevented us from offlining the last CPU - $i"
			fi
		else
			if ! offline_cpu $i; then
				tst_brkm TFAIL "Could not offline $i"
			fi
		fi
	done

	LOOP_COUNT=$((LOOP_COUNT+1))

done

tst_resm TPASS "Success"

tst_exit
