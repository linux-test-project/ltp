#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2003
# Copyright (c) Linux Test Project, 2016-2021
# Written by Prakash Narayana (prakashn@us.ibm.com)
# and Michael Reed (mreed10@us.ibm.com)
#
# Test isofs on Linux system.
# It makes ISO9660 file system with different options and also
# mounts ISO9660 file system with different mount options.

TST_NEEDS_CMDS="mount umount"
TST_NEEDS_TMPDIR=1
TST_SETUP=setup
TST_TESTFUNC=do_test

MAX_DEPTH=3
MAX_DIRS=4

setup()
{
	if tst_cmd_available mkisofs; then
		MKISOFS_CMD="mkisofs"
	elif tst_cmd_available genisoimage; then
		MKISOFS_CMD="genisoimage"
	else
		tst_brk TCONF "please install mkisofs or genisoimage"
	fi
}

gen_fs_tree()
{
	local cur_path="$1"
	local cur_depth="$2"
	local i new_path

	[ "$cur_depth" -gt "$MAX_DEPTH" ] && return

	for i in $(seq 1 $MAX_DIRS); do
		new_path="$cur_path/subdir_$i"
		mkdir -p "$new_path"
		ROD_SILENT dd if=/dev/urandom of="$new_path/file" bs=1024 count=100
		gen_fs_tree "$new_path" $((cur_depth + 1))
	done
}

do_test()
{
	local mnt_point="$PWD/mnt"
	local make_file_sys_dir="$PWD/files"
	local mkisofs_opt mount_opt

	mkdir -p -m 777 $mnt_point
	mkdir -p $make_file_sys_dir

	# Generated directories and files
	mkdir -p $make_file_sys_dir
	gen_fs_tree "$make_file_sys_dir" 1

	# Make ISO9660 file system with different options.
	# Mount the ISO9660 file system with different mount options.
	for mkisofs_opt in \
		" " \
		"-J" \
		"-hfs -D" \
		" -R " \
		"-R -J" \
		"-f -l -D -J -allow-leading-dots -R" \
		"-allow-lowercase -allow-multidot -iso-level 3 -f -l -D -J -allow-leading-dots -R"
	do
		rm -f isofs.iso
		EXPECT_PASS $MKISOFS_CMD -o isofs.iso -quiet $mkisofs_opt $make_file_sys_dir 2\> /dev/null \
			|| continue

		for mount_opt in \
			"loop" \
			"loop,norock" \
			"loop,nojoliet" \
			"loop,block=512,unhide" \
			"loop,block=1024,cruft" \
			"loop,block=2048,nocompress" \
			"loop,check=strict,map=off,gid=bin,uid=bin" \
			"loop,check=strict,map=acorn,gid=bin,uid=bin" \
			"loop,check=relaxed,map=normal" \
			"loop,block=512,unhide,session=2"
		do
			EXPECT_PASS mount -t iso9660 -o $mount_opt isofs.iso $mnt_point \
				|| continue

			EXPECT_PASS ls -lR $mnt_point \> /dev/null
			EXPECT_PASS_BRK umount $mnt_point

			tst_res TPASS "mount/umount with \"$mount_opt\" options"
		done
	done
}

. tst_test.sh
tst_run
