#!/bin/sh
insmod -f ./tmod.ko || insmod -f ./tmod.o || exit 1
rm -f /dev/tmod
major=`awk '/tmod/{print \$1}' /proc/devices`
mknod /dev/tmod c $major 0
