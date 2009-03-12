#!/bin/sh

cd /tmp
DEV=`df . |grep ^/|awk '{print $1}'` 
mount ${DEV} -o remount,mand || exit 2
ftruncate04_64
ret=$?
mount ${DEV} -o remount || exit 2
exit $ret
