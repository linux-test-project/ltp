#!/bin/sh -ex

conf=${1}
arch=$(uname -m)
kver=$(uname -r)

. "${conf}"

echo "Verify Kernel version >= 2.6.16."
# Kernel might in the following format.
# x.y.z-1.el
# x.y.z.1.el
kx=${kver%%.*}
tmp=${kver#*.}
ky=${tmp%%.*}
tmp=${tmp#*.}
tmp=${tmp%%.*}
kz=${tmp%%-*}

if [ "${kx}" -lt 2 ]; then
    error=1

elif [ "${kx}" -eq 2 ]; then
    if [ "${ky}" -lt 6 ]; then
        error=1

    elif [ "${ky}" -eq 6 ]; then
        if [ "${kz}" -lt 16 ]; then
            error=1
        fi
    fi
fi

if [ "${error}" ]; then
    echo "Fail: kernel version ${kver} is less than 2.6.16."
fi


echo "Verify user is root."
if [ $(id -u) != 0 ]; then
    echo "Fail: root is required."
    error=1
fi


echo "Verify prerequisite."
if [ ! -x "/sbin/kexec" ]; then
    echo "Fail: kexec-tools not found."
    error=1
fi

if [ ! -d "/lib/modules/${kver}/build" ]; then
    echo "Fail: kernel-devel not found."
    error=1
fi

if [ "${CRASH}" ] && [ "${CRASH}" -eq 1 ]; then
    if [ ! -x "/usr/bin/crash" ]; then
        echo "Fail: crash not found."
        error=1
    fi

    if [ ! -f "${VMLINUX}" ]; then
        echo "Fail: kernel-debuginfo not found."
        error=1
    fi
fi

# Final result.
if [ "${error}" ]; then
    echo "Please fixed the above failures before continuing."
    exit 1
fi

echo "Compile Kernel modules."
make clean

# Test if struct kprobe has "symbol_name" field.
if make -C kprobes >/dev/null 2>&1; then
    export USE_SYMBOL_NAME=1
fi

make

echo "Modify Boot Loader."
if [ "${arch}" = "ppc64" ]; then
    args="crashkernel=256M@32M xmon=off"
elif [ "${arch}" = "i686" ]; then
    args="crashkernel=256M@128M nmi_watchdog=1"
elif [ "${arch}" = "ia64" ]; then
    args="crashkernel=512M@256M"
else
    args="crashkernel=256M@128M"
fi

if [ -x "/sbin/grubby" ]; then
    /sbin/grubby --default-kernel |
     xargs /sbin/grubby --args="${args}" --update-kernel

else
    echo "Warn: please make sure the following arguments are in Boot\
 Loader:"
    echo "$args"
    echo "Hit any key when ready."
    read
fi

exit 0
