#!/bin/sh
#
# ---
# env
# {
#  "needs_root": true,
#  "mount_device": true,
#  "all_filesystems": true,
#  "mntpoint": "ltp_mntpoint"
# }
# ---

. tst_loader.sh

tst_res TINFO "In shell"

mntpath=$(realpath ltp_mntpoint)
mounted=$(grep $mntpath /proc/mounts)

if [ -n "$mounted" ]; then
	device=$(echo $mounted |cut -d' ' -f 1)
	path=$(echo $mounted |cut -d' ' -f 2)

	tst_res TPASS "$device mounted at $path"
else
	tst_res TFAIL "Device not mounted!"
fi
