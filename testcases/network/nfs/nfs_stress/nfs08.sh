#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2023 Petr Vorel <pvorel@suse.cz>
# Test for broken NFS cache invalidation for directories.
# Kernel patch broke cache invalidation, which caused the second 'ls'
# not shown '2'.
# https://lore.kernel.org/linux-nfs/167649314509.15170.15885497881041431304@noble.neil.brown.name/
# Based on reproducer from Neil Brown <neilb@suse.de>

TST_TESTFUNC="do_test"

do_test()
{
	tst_res TINFO "testing NFS cache invalidation for directories"

	touch 1
	EXPECT_PASS 'ls | grep 1'
	touch 2
	EXPECT_PASS 'ls | grep 2'
}

. nfs_lib.sh
tst_run
