#!/bin/sh
insmod -f ./tagp.ko || insmod -f ./tagp.o || exit 1
rm -f /dev/tagp
major=`awk '/tagp/{print \$1}' /proc/devices`
mknod /dev/tagp c $major 0
