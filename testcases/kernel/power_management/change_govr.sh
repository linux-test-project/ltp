#!/bin/bash

. pm_include.sh
available_govr=$(get_supporting_govr)

total_cpus=$(no_of_cpus)
(( total_cpus-=1 ))
RC=0

for cpu in $(seq 0 "${total_cpus}" )
do
	for govr in ${available_govr}
	do
		echo ${govr} > /sys/devices/system/cpu/cpu${cpu}/cpufreq/scaling_governor
		if [ "$?" -ne "0" ] ; then
			echo "${0}: FAIL: Unable to set governor -- ${govr} for cpu${cpu}"
			RC=1
		fi
	done
done
if [ ${RC} -eq 0 ] ; then
	echo "${0}: PASS: Changing cpu governors"
fi
exit $RC
