#!/bin/bash

. pm_include.sh

total_cpus=$(no_of_cpus)
(( total_cpus-=1 ))
RC=0

for cpu in $(seq 0 "${total_cpus}" )
do
	cpufiles=$(find /sys/devices/system/cpu/cpu"${cpu}"/cpufreq/ -name "*" -type f)
	for files in ${cpufiles}
	do
		cat ${files} 2>&1 >/dev/null
		if [ $? -ne 0 ] ; then
			echo "${0}: FAIL: cat ${files}"
			RC=1
		fi
	done
done
if [ ${RC} -eq 0 ] ; then
	echo "${0}: PASS: Checking cpu freq sysfs files"
fi
exit $RC
