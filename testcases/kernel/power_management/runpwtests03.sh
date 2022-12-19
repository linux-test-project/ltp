#! /bin/sh
#
# Copyright (c) International Business Machines  Corp., 2001
# Author: Nageswara R Sastry <nasastry@in.ibm.com>
#
# This program is free software;  you can redistribute it and#or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program;  if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

export TCID="Power_Management03"
export TST_TOTAL=4

. test.sh
. pm_include.sh

check_cpufreq_sysfs_files() {
	total_cpus=`expr $(tst_ncpus) - 1`
	RC=0

	for cpu in $(seq 0 "${total_cpus}" )
	do
		cpufiles=$(find /sys/devices/system/cpu/cpu"${cpu}"/cpufreq/ \
			-name "*" -type f -perm /400)
		for files in ${cpufiles}
		do
			cat ${files} >/dev/null 2>&1
			if [ $? -ne 0 ] ; then
				echo "${0}: FAIL: cat ${files}"
				RC=1
			fi
		done
	done
	if [ ${RC} -eq 0 ] ; then
		echo "${0}: PASS: Checking cpu freq sysfs files"
	fi
	return $RC
}

change_govr() {
	available_govr=$(get_supporting_govr)

	total_cpus=`expr $(tst_ncpus) - 1`
	RC=0

	for cpu in $(seq 0 "${total_cpus}" )
	do
		for govr in ${available_govr}
		do
			echo ${govr} > \
	/sys/devices/system/cpu/cpu${cpu}/cpufreq/scaling_governor
			if [ "$?" -ne "0" ] ; then
				echo "${0}: FAIL: Unable to set" \
					"governor -- ${govr} for cpu${cpu}"
				RC=1
			fi
		done
	done
	if [ ${RC} -eq 0 ] ; then
		echo "${0}: PASS: Changing cpu governors"
	fi
	return $RC
}

change_freq() {
	available_freq=$(get_supporting_freq)
	available_govr=$(get_supporting_govr)
	RC=0

	total_cpus=`expr $(tst_ncpus) - 1`

	if ( echo ${available_govr} | grep -i "userspace" \
		>/dev/null 2>&1 ); then
		for cpu in $(seq 0 "${total_cpus}" )
		do
			echo userspace > \
	/sys/devices/system/cpu/cpu${cpu}/cpufreq/scaling_governor
			if [ $? -ne 0 ] ; then
				RC=1
			fi
		done
		if [ ${RC} -ne 1 ] ; then
			for cpu in $(seq 0 "${total_cpus}" )
			do
				for freq in ${available_freq}
				do
					echo ${freq} > \
	/sys/devices/system/cpu/cpu${cpu}/cpufreq/scaling_setspeed
					if [ "$?" -ne "0" ] ; then
						echo "${0}: FAIL: Unable" \
							"to set frequency -- ${freq} for cpu${cpu}"
						RC=1
					fi
				done
			done
		fi
	fi
	if [ ${RC} -eq 0 ] ; then
		echo "${0}: PASS: Changing cpu frequencies"
	fi
	return $RC
}

pwkm_load_unload() {
	RC=0
	loaded_governor=`cat \
		/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`
	for module in `modprobe -l | grep cpufreq_ | \
		cut -f8 -d"/" | cut -f1 -d"."`
	do
		#echo -n "Loading $module ... "
		if [ $module != "cpufreq_$loaded_governor" ]; then
			modprobe $module >/dev/null
			if [ $? -ne 0 ] ; then
				echo "${0}: FAIL: Loading of module $module" \
					"or check whether you compiled as module or not"
				RC=1
			fi
		fi
	done
	for module in `modprobe -l | grep cpufreq_ | \
		cut -f8 -d"/" | cut -f1 -d"."`
		do
		#echo -n "Unloading $module ... "
		if [ $module != "cpufreq_$loaded_governor" ]; then
			modprobe -r $module >/dev/null
			if [ $? -ne 0 ] ; then
				echo "${0}: FAIL: Loading of module $module" \
					"or check whether you compiled as module or not"
				RC=1
			fi
		fi
	done
	return $RC
}

# Checking test environment
check_arch

# Checking cpufreq sysfs interface files
if [ ! -d /sys/devices/system/cpu/cpu0/cpufreq ] ; then
	tst_brkm TCONF "Required kernel configuration for CPU_FREQ NOT set"
fi

if check_cpufreq_sysfs_files ; then
	tst_resm TPASS "CPUFREQ sysfs tests"
else
	tst_resm TFAIL "CPUFREQ sysfs tests"
fi

# Changing governors
if change_govr ; then
	tst_resm TPASS "Changing governors"
else
	tst_resm TFAIL "Changing governors"
fi

# Changing frequencies
if change_freq ; then
	tst_resm TPASS "Changing frequncies"
else
    tst_resm TFAIL "Changing frequncies"
fi

# Loading and Unloading governor related kernel modules
if pwkm_load_unload ; then
	tst_resm TPASS "Loading and Unloading of governor kernel" \
		"modules"
else
	tst_resm TFAIL "Loading and Unloading of governor kernel" \
		"modules got failed"
fi

tst_exit
