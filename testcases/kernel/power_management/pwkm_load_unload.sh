#!/bin/bash

#Loading modules
for module in cpufreq_conservative cpufreq_powersave cpufreq_userspace
do
	#echo -n "Loading $module ... "
	modprobe $module >/dev/null
	if [ $? -ne 0 ] ; then
		echo "${0}: FAIL: Loading of module $module or check whether you compiled as module or not"
	fi
done
for module in cpufreq_conservative cpufreq_powersave cpufreq_userspace
do
	#echo -n "Unloading $module ... "
	modprobe -r $module >/dev/null
	if [ $? -ne 0 ] ; then
		echo "${0}: FAIL: Loading of module $module or check whether you compiled as module or not"
	fi
done
