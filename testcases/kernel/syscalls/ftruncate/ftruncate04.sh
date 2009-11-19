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
    FTRUNCATE=$(readlink -f "$(dirname "$0")/ftruncate04${1+_${1}}")
    if [ ! -x "$FTRUNCATE" ] ; then
        error "$FTRUNCATE is not executable"
        exit 1
    fi
fi
cd ${TMPDIR:=/tmp}
DEV=$(df . | awk '/^\// { print $1 }')
MOUNT_POINT=$(df . | tail -n 1 | awk '{ print $NF }')
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
