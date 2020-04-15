#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
#
# Clean up LVM volume groups created by prepare_lvm.sh

TST_TESTFUNC=cleanup_lvm
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="losetup umount vgremove"
. tst_test.sh

LVM_TMPDIR="/tmp/ltp/growfiles"
LVM_IMGDIR="/tmp/ltp/imgfiles"

cleanup_lvm()
{
	DEVLIST=`losetup -lnO NAME,BACK-FILE | grep "$LVM_IMGDIR" | cut -d ' ' -f 1`

	for dir in "$LVM_TMPDIR/"*; do
		tst_umount $dir
	done

	ROD vgremove -y ltp_test_vg1
	ROD vgremove -y ltp_test_vg2

	for devname in $DEVLIST; do
		ROD tst_device release $devname
	done

	rm -rf /tmp/ltp
	tst_res TPASS "LVM configuration for LTP removed successfully."
}

tst_run
