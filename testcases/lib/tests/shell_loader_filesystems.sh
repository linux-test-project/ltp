#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>
#
# ---
# env
# {
#  "mount_device": true,
#  "runtime": 10,
#  "mntpoint": "ltp_mntpoint",
#  "filesystems": [
#   {
#    "type": "tmpfs"
#   },
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

tst_test()
{
	tst_res TINFO "In shell"

	mntpoint=$(realpath ltp_mntpoint)
	mounted=$(grep $mntpoint /proc/mounts)

	if [ -n "$mounted" ]; then
		fs=$(echo $mounted |cut -d' ' -f 3)

		tst_res TPASS "Mounted device formatted with $fs"
	else
		tst_res TFAIL "Device not mounted!"
	fi

	RUNTIME=$(tst_remaining_runtime)

	if [ "$RUNTIME" -ge 9 ]; then
		tst_res TPASS "Remaining runtime $RUNTIME"
	else
		tst_res TFAIL "Remaoning runtime $RUNTIME"
	fi
}

. tst_run.sh
