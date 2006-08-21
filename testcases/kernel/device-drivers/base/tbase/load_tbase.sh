#!/bin/sh
insmod -f ./tbase.ko || insmod -f ./tbase.o || exit 1
rm -f /dev/tbase
major=`awk '/tbase/{print \$1}' /proc/devices`
mknod /dev/tbase c $major 0
