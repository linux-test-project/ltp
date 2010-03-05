#!/bin/sh
#
# Test Case 4
#

HOTPLUG04_LOOPS=${HOTPLUG04_LOOPS:-${LOOPS}}
export TCID="hotplug04"
export TST_COUNT=1
export TST_TOTAL=${HOTPLUG04_LOOPS:-1}

# Includes:
LHCS_PATH=${LHCS_PATH:-${LTPROOT:+$LTPROOT/testcases/bin/cpu_hotplug}}
. $LHCS_PATH/include/testsuite.fns
. $LHCS_PATH/include/hotplug.fns

cat <<EOF
Name:   $TCID
Date:   `date`
Desc:   Does it prevent us from offlining the last CPU?

EOF

cpu=0
until [ $TST_COUNT -gt $TST_TOTAL ]; do
	cpustate=1

	# Online all the CPUs' keep track of which were already on
	for i in $(get_all_cpus); do
		online_cpu $i
		RC=$?
		if [ $RC -ne 0 ]; then
			: $(( cpu += 1 ))
			eval "on_${cpu}=$i"
			echo $i
		fi
		if [ $RC -eq 0 -a "$i" = "cpu0" ]; then
			cpustate=0
		fi
	done

	# Now offline all the CPUs
	for i in $(get_all_cpus); do
		offline_cpu $i
		RC=$?
		if [ $RC -eq 1 ]; then
			if [ "x$i" != "xcpu0" ]; then
				tst_resm TFAIL "Did not offline first CPU (offlined $i instead)"
			else
				tst_resm TPASS "Successfully offlined first CPU, $i"
			fi
		fi
	done

	# Online the ones that were on initially
	until [ $cpu -eq 0 ]; do
		online_cpu $(eval "echo \$on_${cpu}")
		: $(( cpu -= 1 ))
	done

	# Return CPU 0 to its initial state
	if [ $cpustate = 1 ]; then
		online_cpu 0
	else 
		offline_cpu 0
	fi

	: $(( TST_COUNT += 1 ))

done

exit_clean
