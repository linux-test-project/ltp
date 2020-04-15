#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 SUSE LLC <mdoucha@suse.cz>
#
# Generate LTP runfile for LVM tests (runtest/lvm.local)

TST_TESTFUNC=generate_runfile
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="sed"
. tst_test.sh

generate_runfile()
{
	trap 'tst_brk TBROK "Cannot create LVM runfile"' ERR
	INFILE="$LTPROOT/testcases/data/lvm/runfile.tpl"
	OUTFILE="$LTPROOT/runtest/lvm.local"
	FS_LIST=`tst_supported_fs`
	echo -n "" >"$OUTFILE"

	for fsname in $FS_LIST; do
		# Btrfs needs too much space for reliable stress testing
		if [ "x$fsname" != "xbtrfs" ]; then
			sed -e "s/{fsname}/$fsname/g" "$INFILE" >>"$OUTFILE"
		fi
	done

	tst_res TPASS "Runfile $OUTFILE successfully created"
}

tst_run
