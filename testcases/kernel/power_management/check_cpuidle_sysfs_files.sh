#!/bin/bash

RC=0
if [ -d /sys/devices/system/cpu/cpuidle ] ; then
	for files in current_governor_ro current_driver
	do
		cat /sys/devices/system/cpu/cpuidle/${files} >/dev/null 2>&1
		if [ $? -ne 0 ] ; then
			echo "${0}: FAIL: cat ${files}"
			RC=1
		fi
	done
fi
if [ ${RC} -eq 0 ] ; then
	echo "${0}: PASS: Checking cpu idle sysfs files"
else
	echo "${0}: FAIL: Checking cpu idle sysfs files"
fi
exit $RC
