#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024-2025 Cyril Hrubis <chrubis@suse.cz>
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

tst_test()
{
	local mntpath="$(realpath ltp_mntpoint)"
	local mounted="$(grep $mntpath /proc/mounts)"
	local device path

	tst_res TINFO "In shell"

	if [ -n "$mounted" ]; then
		device=$(echo $mounted |cut -d' ' -f 1)
		path=$(echo $mounted |cut -d' ' -f 2)

		tst_res TPASS "$device mounted at $path"
	else
		tst_res TFAIL "Device not mounted!"
	fi
}

. tst_run.sh
