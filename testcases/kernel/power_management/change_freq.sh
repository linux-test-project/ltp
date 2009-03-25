#!/bin/bash

. pm_include.sh
available_freq=$(get_supporting_freq)
available_govr=$(get_supporting_govr)
RC=0

total_cpus=$(no_of_cpus)
(( total_cpus-=1 ))

if ( echo ${available_govr} | grep -i "userspace" 2>&1 >/dev/null ) ; then
	for cpu in $(seq 0 "${total_cpus}" )
	do
		echo userspace > /sys/devices/system/cpu/cpu${cpu}/cpufreq/scaling_governor
		if [ $? -ne 0 ] ; then
			RC=1
		fi
	done
	if [ ${RC} -ne 1 ] ; then
		for cpu in $(seq 0 "${total_cpus}" )
		do
			for freq in ${available_freq}
			do
				echo ${freq} > /sys/devices/system/cpu/cpu${cpu}/cpufreq/scaling_setspeed
				if [ "$?" -ne "0" ] ; then
					echo "${0}: FAIL: Unable to set frequency -- ${freq} for cpu${cpu}"
					RC=1
				fi
			done
		done
	fi
fi
if [ ${RC} -eq 0 ] ; then
	echo "${0}: PASS: Changing cpu frequencies"
fi
exit $RC
