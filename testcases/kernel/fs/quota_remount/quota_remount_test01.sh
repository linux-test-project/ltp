#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Jan Kara <jack@suse.cz>, 2008
# Copyright (c) International Business Machines  Corp., 2009
# Copyright (c) Köry Maincent <kory.maincent@bootlin.com> 2021
# Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>

TST_NEEDS_CMDS="dd mkfs.ext3 mount quota quotacheck quotaon sed tail"
TST_NEEDS_DRIVERS="quota_v2"
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_SETUP=do_setup
TST_CLEANUP=do_clean
TST_TESTFUNC=do_test
TST_MIN_KVER="2.6.26"

do_setup()
{
	if [ ! -d /proc/sys/fs/quota ]; then
		tst_brk TCONF "quota not supported in kernel"
	fi

	MNTDIR="mnt.$$"
	IMAGE="ltp-$$-fs-image"
	ROD dd if=/dev/zero of=$IMAGE bs=4096 count=8000 2>/dev/null
	ROD mkfs.ext3 -q -F -b 4096 $IMAGE
	mkdir $MNTDIR
}

do_clean()
{
	[ "$mounted" ] || return
	tst_umount "$PWD/$MNTDIR"
	mounted=
}

get_blocks()
{
	quota -f $MNTDIR -v -w | tail -n 1 | sed -e 's/ *[^ ]* *\([0-9]*\) .*/\1/'
}

do_test()
{
	tst_res TINFO "testing quota on remount"

	local blocks newblocks

	ROD mount -t ext3 -o loop,usrquota,grpquota $IMAGE $MNTDIR
	mounted=1

	# some distros (CentOS 6.x, for example) doesn't permit creating
	# of quota files in a directory with SELinux file_t type
	if tst_selinux_enforced &&
		tst_cmd_available chcon && ! chcon -t tmp_t $MNTDIR; then
			tst_brk TCONF "could not change SELinux file type"
	fi

	ROD quotacheck -cug $MNTDIR
	ROD quotaon -ug $MNTDIR
	ROD echo "blah" />$MNTDIR/file

	blocks=$(get_blocks)
	ROD mount -o remount,ro $MNTDIR
	ROD mount -o remount,rw $MNTDIR

	ROD rm $MNTDIR/file
	newblocks=$(get_blocks)

	if [ $blocks -eq $newblocks ]; then
	   tst_brk TFAIL "usage did not change after remount"
	fi

	tst_res TPASS "quota on remount passed"

	do_clean
}

. tst_test.sh
tst_run
