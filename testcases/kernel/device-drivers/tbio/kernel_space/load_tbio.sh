#!/bin/sh
insmod -f ./tbio.ko || insmod -f ./tbio.o || exit 1
rm -f /dev/tbio0
major=`awk '/tbio/{print \$1}' /proc/devices`
mknod /dev/tbio0 b $major 0
