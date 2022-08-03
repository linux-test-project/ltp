#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
#
# Create and mount LVM volume groups for lvm.local runfile

TST_TESTFUNC=prepare_lvm
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="mount pvcreate vgcreate lvcreate"

LVM_DIR="${LVM_DIR:-/tmp}"
LVM_TMPDIR="$LVM_DIR/ltp/growfiles"
LVM_IMGDIR="$LVM_DIR/ltp/imgfiles"

error_check()
{
	if [ $? -ne 0 ]; then
		tst_brk TBROK "LVM setup failed"
	fi
}

create_volume()
{
	fsname=$2
	ROD mkdir -p $fsname

	# If the FS isn't supported, only create the mountpoint and exit
	if ! tst_supported_fs $fsname; then
		return
	fi

	vgname=$1
	lvname="ltp_lv_$fsname"
	lvdev="/dev/$vgname/$lvname"

	ROD lvcreate -L 1G $vgname -n "$lvname"
	tst_mkfs $fsname "$lvdev"
	ROD mount "$lvdev" $fsname
}

prepare_mounts()
{
	FSNAME1=$1
	FSNAME2=$2
	shift 2
	LVM_DEV1=`tst_device acquire 1040 "$LVM_IMGDIR/lvm_pv1.img"`
	error_check
	LVM_DEV2=`tst_device acquire 1040 "$LVM_IMGDIR/lvm_pv2.img"`
	error_check

	# DEVSIZE=($# * 1GB / 2) + 16MB. The extra 16MB is for LVM physical
	# volume headers
	DEVSIZE=$(( $# * 512 + 16 ))
	LVM_DEV3=`tst_device acquire $DEVSIZE "$LVM_IMGDIR/lvm_pv3.img"`
	error_check
	LVM_DEV4=`tst_device acquire $DEVSIZE "$LVM_IMGDIR/lvm_pv4.img"`
	error_check
	ROD pvcreate $LVM_DEV1 $LVM_DEV2 $LVM_DEV3 $LVM_DEV4
	ROD vgcreate ltp_test_vg1 $LVM_DEV1 $LVM_DEV2
	ROD vgcreate ltp_test_vg2 $LVM_DEV3 $LVM_DEV4

	for fsname in $FSNAME1 $FSNAME2; do
		create_volume ltp_test_vg1 $fsname
	done

	for fsname in $@; do
		create_volume ltp_test_vg2 $fsname
	done
}

prepare_lvm()
{
	FS_LIST=$(tst_supported_fs -s tmpfs | sort -u)
	ROD mkdir -p "$LVM_TMPDIR"
	ROD mkdir -p "$LVM_IMGDIR"
	chmod 777 "$LVM_TMPDIR"
	cd "$LVM_TMPDIR"
	error_check
	prepare_mounts $FS_LIST
	tst_res TPASS "LVM mounts are ready"
}

. tst_test.sh
tst_run
