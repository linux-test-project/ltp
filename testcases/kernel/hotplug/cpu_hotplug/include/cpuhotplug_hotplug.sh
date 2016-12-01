#!/bin/sh

# cpuhotplug_hotplug.sh - Collection of functions for hotplugging
# operations.

# Routines in this library are set up to allow timing to be done
# by defining $TIME to a timing command.
TIME=${TIME:-""}

# get_all_irqs()
#
#  Gets list of all available IRQs in the system
#
get_all_irqs()
{
    echo `egrep [0-9]+: /proc/interrupts | cut -d ':' -f 1`
    return
}

# migrate_irq(CPU, IRQS)
#
#  Sets the smp_affinity for the list of $IRQS to the given
#  CPU number
#
migrate_irq()
{
    CPU=${1#cpu}
    MASK=$((1<<${CPU}))
    IRQS=$2
    for irq in ${IRQS}; do
      echo $MASK > /proc/irq/${irq}/smp_affinity || \
        tst_resm TINFO "It is NOT permitted to change the IRQ $irq smp_affinity"
    done
}


# get_affinity(PID)
#
#  Echos the CPU affinity for the given process ID to stdout
#
get_affinity_mask()
{
    AFFINITY=`taskset -p ${1}`
    echo ${AFFINITY}
    return
}

# set_affinity(PID, CPU)
#
#  Sets the affinity for the given PID to the specified CPU.
#
set_affinity()
{
    PID="$1"
    CPU="$2"
    MASK=$((1<<${CPU_TO_TEST}))
    `taskset -p ${MASK} ${PID} > /dev/null 2>&1`
    return $?
}

# online_cpu(CPU)
#
#  Onlines the given CPU.  Returns a true value if it was able
#  to perform the online operation successfully, false otherwise.
#
#  $CPU should either be a specific number like 4, or the cpu name,
#  as in 'cpu4'.
#
online_cpu()
{
    CPU=${1#cpu}
    if [ ! -w /sys/devices/system/cpu/cpu${CPU}/online ]; then
        return 1
    fi

    cpu_is_online ${CPU} && return 0

    $TIME echo 1 > /sys/devices/system/cpu/cpu${CPU}/online
    RC=$?
    report_timing "Online cpu ${CPU}"
    return $RC
}


# offline_cpu(CPU)
#
#  Offlines the given CPU.  Returns a true value if it was able
#  to perform the offline operation successfully, false otherwise.
#
offline_cpu()
{
    CPU=${1#cpu}
    if [ ! -w /sys/devices/system/cpu/cpu${CPU}/online ]; then
        return 1
    fi

    ! cpu_is_online ${CPU} && return 0

    $TIME echo 0 > /sys/devices/system/cpu/cpu${CPU}/online
    RC=$?
    report_timing "Offline cpu ${CPU}"
    return $RC
}

# get_cpus_num()
#
#  Prints the number of all available CPUs, regardless of whether they're
#  currently online or offline.
#
get_cpus_num()
{
	[ -d /sys/devices/system/cpu/cpu0 ] || return -1
	NUM=`ls /sys/devices/system/cpu/ \
			| grep -c "cpu[0-9][0-9]*"`
	return $NUM
}

# get_all_cpus()
#
#  Prints a list of all available CPUs, regardless of whether they're
#  currently online or offline.
#
#  This routine will work even if the CPUs are not hotpluggable, however
#  it requires you have sysfs enabled in the kernel.
#
get_all_cpus()
{
    [ -d /sys/devices/system/cpu ] || return 1
    (cd /sys/devices/system/cpu; ls -d cpu[0-9]*)
}

# get_present_cpus()
#
#  Prints a list of present CPUs, regardless of whether they're
#  currently online or offline.
#
get_present_cpus()
{
    local present_mask="/sys/devices/system/cpu/present"
    local present_cpus=""

    # if sysfs present mask is missing, assume all cpu are present
    if [ ! -e "$present_mask" ]; then
        get_all_cpus
        return
    fi

    for part in $(cat $present_mask | tr "," " "); do
        if echo $part | grep -q "-"; then
            range_low=$(echo $part | cut -d - -f 1)
            range_high=$(echo $part | cut -d - -f 2)
        else
            range_low=$part
            range_high=$part
        fi
        for cpu in $(seq $range_low $range_high); do
            if [ -e /sys/devices/system/cpu/cpu$cpu ]; then
                present_cpus="$present_cpus cpu$cpu"
            fi
        done
    done
    echo $present_cpus
}

# get_present_cpus_num()
#
#  Prints the number of present CPUs
#
get_present_cpus_num()
{
    echo $(get_present_cpus | wc -w)
}

# get_hotplug_cpus()
#
#  Prints a list of present hotpluggable CPUs, regardless of whether they're
#  currently online or offline.
#
get_hotplug_cpus()
{
    local present_cpus="$(get_present_cpus)"
    local hotplug_cpus=""

    for cpu in $present_cpus; do
        if [ -e /sys/devices/system/cpu/$cpu/online ]; then
            hotplug_cpus="$hotplug_cpus $cpu"
	fi
    done
    echo $hotplug_cpus
}

# get_hotplug_cpus_num()
#
#  Prints the number of hotpluggable CPUs
#
get_hotplug_cpus_num()
{
    echo $(get_hotplug_cpus | wc -w)
}

# get_all_cpu_states()
#
#  Collects the current online/offline state of CPUs in the
#  system, printing it in a format that can be passed to
#  set_all_cpu_states() later.
#
get_all_cpu_states()
{
    echo `cd /sys/devices/system/cpu/ && grep '' */online | \
		sed -e 's/\/online//'`
    return
}

# set_all_cpu_states(STATES)
#
#  Sets all of the CPU states according to STATES, which must be
#  of the form "cpuX:Y", where X is the CPU number and Y its state.
#  Each must be on a separate line.
#
set_all_cpu_states()
{
    for cpu_state in $1; do
        cpu=`echo $cpu_state | cut -d: -f 1`
        state=`echo $cpu_state | cut -d: -f 2`
        if [ $state = 1 ]; then
            online_cpu $cpu
        else
            offline_cpu $cpu
        fi
    done
}


# get_online_cpus()
#
#  Prints a list of all CPUs currently online.  This function only
#  works if the system's CPUs have hotplug capabilities
#
get_online_cpus()
{
    echo `cd /sys/devices/system/cpu/ && grep 1 */online | cut -d '/' -f 1`
    return
}


# get_offline_cpus()
#
#  Prints a list of all CPUs currently offline.  This function only
#  works if the system's CPUs have hotplug capabilities
#
get_offline_cpus()
{
    echo `cd /sys/devices/system/cpu/ && grep 0 */online | cut -d '/' -f 1`
    return
}

# cpu_is_valid(CPU)
#
#  Checks to see if the given CPU number is available for hotplugging
#  in the system.  Returns 0 if the CPU is available, 1 otherwise.
#
cpu_is_valid()
{
    CPU=${1#cpu}
    echo "CPU is $CPU"
    cat /sys/devices/system/cpu/cpu${CPU}/online > /dev/null 2>&1
    return $?
}


# cpu_is_online(CPU)
#
#  Returns a 0 value if the given CPU number is currently online,
#  1 otherwise.  This function requires the system's CPUs have
#  hotplug capabilities.
#
cpu_is_online()
{
    CPU=${1#cpu}
    if [ `cat /sys/devices/system/cpu/cpu${CPU}/online` = "1" ]; then
        return 0
    else
        return 1
    fi
}
