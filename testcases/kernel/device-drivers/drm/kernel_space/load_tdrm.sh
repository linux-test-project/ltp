#!/bin/sh
#rmmod -f tdrm.o
insmod -f ./tdrm.ko || insmod -f ./tdrm.o || exit 1
rm -f /dev/tdrm
major=`awk '/drm/{print \$1}' /proc/devices`
minor=`dmesg | grep  "Initialized tdrm" | tail -n 1 | awk '{print \$NF}'`
mknod /dev/tdrm c $major $minor
