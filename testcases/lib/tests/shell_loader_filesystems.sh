#!/bin/sh
#
# ---
# env
# {
#  "mount_device": true,
#  "mntpoint": "ltp_mntpoint",
#  "filesystems": [
#   {
#    "type": "btrfs"
#   },
#   {
#    "type": "xfs",
#    "mkfs_opts": ["-m", "reflink=1"]
#   }
#  ]
# }
# ---

. tst_loader.sh

tst_res TINFO "In shell"

mntpoint=$(realpath ltp_mntpoint)
mounted=$(grep $mntpoint /proc/mounts)

if [ -n "$mounted" ]; then
	fs=$(echo $mounted |cut -d' ' -f 3)

	tst_res TPASS "Mounted device formatted with $fs"
else
	tst_res TFAIL "Device not mounted!"
fi
