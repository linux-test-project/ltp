#!/bin/sh

cd /tmp
DEV=`df . |grep ^/|awk '{print $1}'` 
MOUNT_POINT=`df .| tail -1 |awk '{print $NF}'`
FLAG=`mount| grep ${DEV} | sed 's/.*(\(.*\)).*/\1/g'`
mount ${DEV} -o remount,mand || { echo "the ${DEV} remount,mand failed" ; exit 2;}
ftruncate04_64
ret=$?
mount -o remount,${FLAG}  ${DEV} ${MOUNT_POINT} || { echo "the ${DEV} remount,${FLAG} failed" ; exit 2; }
exit $ret
