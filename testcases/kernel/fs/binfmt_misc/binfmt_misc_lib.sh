#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
# Author: Xiao Yang <yangx.jy@cn.fujitsu.com>

TST_SETUP="${TST_SETUP:-binfmt_misc_setup}"
TST_CLEANUP="${TST_CLEANUP:-binfmt_misc_cleanup}"
TST_NEEDS_DRIVERS="binfmt_misc"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="${TST_NEEDS_CMDS} modprobe mount umount mkdir rm"

rmod_binfmt_misc=0
umount_binfmt_misc=0
binfmt_misc_mntpoint="ltp_binfmt_misc"

remove_binary_type()
{
	local name=$1

	(echo -1 >"$name") 2>/dev/null
	[ $? -ne 0 -o -f "$name" ] && \
		tst_res TWARN "Fail to remove a binary type"
}

get_binfmt_misc_mntpoint()
{
	local mntpoint

	mntpoint=$(awk '/ binfmt_misc / { print $2 }' /proc/mounts)
	[ -z "$mntpoint" ] && tst_brk TBROK "Can't get binfmt_misc mntpoint"

	echo "$mntpoint"
}

binfmt_misc_setup()
{
	local mntpoint

	if ! grep -q "binfmt_misc" /proc/filesystems; then
		ROD modprobe binfmt_misc
		rmod_binfmt_misc=1
	fi

	# Match fs type accurately, because autofs is also mounted on
	# /proc/sys/fs/binfmt_misc on some distros, as below:
	# cat /proc/mounts | grep binfmt_misc
	# systemd-1 /proc/sys/fs/binfmt_misc autofs ...
	# binfmt_misc /proc/sys/fs/binfmt_misc binfmt_misc ...
	mntpoint=$(awk '/ binfmt_misc / { print $2 }' /proc/mounts)
	[ -n "$mntpoint" ] && return

	ROD mkdir ${binfmt_misc_mntpoint}
	ROD mount -t binfmt_misc none ${binfmt_misc_mntpoint}
	umount_binfmt_misc=1
}

binfmt_misc_cleanup()
{
	if [ ${umount_binfmt_misc} -ne 0 ]; then
		umount ${binfmt_misc_mntpoint}
		umount_binfmt_misc=0
	fi

	[ -d ${binfmt_misc_mntpoint} ] && rm -rf ${binfmt_misc_mntpoint}

	if [ ${rmod_binfmt_misc} -ne 0 ]; then
		modprobe -r binfmt_misc
		rmod_binfmt_misc=0
	fi
}

. tst_test.sh
