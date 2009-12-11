#!/bin/sh

error() {
    echo "${0##*/} : ERROR : $*"
}

usage() {
    echo "usage: ${0##*/} [64]"
}

if [ $# -gt 1 ] || [ -n "$1" -a "$1" != 64 ] ; then
    usage
    exit 1
else
    FTRUNCATE=ftruncate04${1+_${1}}
    export TCID=$FTRUNCATE
    export TST_COUNT=1
    export TST_TOTAL=1
fi
cd ${TMPDIR:=/tmp}
set -- $(df . | awk '/^\// { print $1, $NF }')
DEV=$1; MOUNT_POINT=$2
if [ "x$DEV" = x -o "x$MOUNT_POINT" = x ] ; then
    tst_resm TCONF "backend mountpoint for $TMPDIR does not correspond to a real device:"
    df .
    exit 0 
fi
FLAG=$(mount | grep ${DEV} | sed 's/.*(\(.*\)).*/\1/g')
cat <<EOF
DEV:         $DEV
MOUNT_POINT: $MOUNT_POINT
FLAG:        $FLAG
EOF
if ! mount ${DEV} -o remount,mand; then
    error "mounting ${DEV} remount,mand failed" 
    exit 2
fi
"$FTRUNCATE"
ret=$?
if ! mount -o remount,${FLAG} ${DEV} ${MOUNT_POINT}; then
    error "mounting ${DEV} remount,${FLAG} failed"
    [ $ret -eq 0 ] || exit 2
fi
exit $ret
