#!/bin/sh -x

conf=${1}; shift
vmcore=${1}; shift
crash=${1}; shift

. "${conf}"

echo ""
echo "----------------------------------------------------------"
echo "                        VMCORE SIZE                       "
echo "----------------------------------------------------------"
echo ""

ls -lh "${vmcore}"

echo ""
echo "----------------------------------------------------------"
echo "                        READELF                           "
echo "----------------------------------------------------------"
echo ""

readelf -a "${vmcore}"

if [ "${crash}" -eq 1 ]; then
    echo ""
    echo "----------------------------------------------------------"
    echo "                        CRASH                             "
    echo "----------------------------------------------------------"
    echo ""

    cat <<EOF >crash_cmd
mod
mod -S
runq
foreach bt
foreach files
mount
mount -f
mount -i
vm
ascii
net
set
set -v
bt
bt -a
bt -f
bt -e
bt -E
ps
ps -k
ps -u
ps -s
dev
dev -p
kmem -i
kmem -s
task
exit
EOF

    crash -i crash_cmd "${VMLINUX}" "${vmcore}"
    rm -f crash_cmd
fi
