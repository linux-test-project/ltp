#!/bin/bash

#Loading modules
loaded_governor=`cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`
for module in cpufreq_conservative cpufreq_powersave cpufreq_userspace
do
	#echo -n "Loading $module ... "
    if [ $module != "cpufreq_$loaded_governor" ];
    then
		modprobe $module >/dev/null
		if [ $? -ne 0 ] ; then
			echo "${0}: FAIL: Loading of module $module or check whether you compiled as module or not"
		fi
	fi
done
for module in cpufreq_conservative cpufreq_powersave cpufreq_userspace
do
	#echo -n "Unloading $module ... "
    if [ $module != "cpufreq_$loaded_governor" ];
    then
		modprobe -r $module >/dev/null
		if [ $? -ne 0 ] ; then
			echo "${0}: FAIL: Loading of module $module or check whether you compiled as module or not"
		fi
	fi
done
