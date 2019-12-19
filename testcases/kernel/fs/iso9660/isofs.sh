#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2003
# Written by Prakash Narayana (prakashn@us.ibm.com)
# and Michael Reed (mreed10@us.ibm.com)
# Copyright (c) Linux Test Project, 2016-2019
#
# Test isofs on Linux system.
# It makes ISO9660 file system with different options and also
# mounts ISO9660 file system with different mount options.

TST_NEEDS_CMDS="mkisofs"
TST_NEEDS_TMPDIR=1
TST_TESTFUNC=do_test
. tst_test.sh

MAX_DEPTH=3
MAX_DIRS=4

gen_fs_tree()
{
	local cur_path="$1"
	local cur_depth="$2"
	local new_path

	[ "$cur_depth" -gt "$MAX_DEPTH" ] && return

	for i in $(seq 1 $MAX_DIRS); do
		new_path="$cur_path/subdir_$i"
		mkdir -p "$new_path"
		ROD_SILENT dd if=/dev/urandom of="$new_path/file" bs=1024 count=100
		gen_fs_tree "$new_path" $((cur_depth + 1))
	done
}

do_test() {
	local mnt_point="$PWD/mnt"
	local make_file_sys_dir="$PWD/files"

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
		EXPECT_PASS mkisofs -o isofs.iso -quiet $mkisofs_opt $make_file_sys_dir 2\> /dev/null \
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
			mount -t iso9660 -o $mount_opt isofs.iso $mnt_point
			if [ $? -ne 0 ]; then
				tst_res TFAIL "mount -t iso9660 -o $mount_opt isofs.iso $mnt_point"
				continue
			fi

			ls -lR $mnt_point > /dev/null || tst_res TFAIL "ls -lR $mnt_point"
			umount $mnt_point || tst_brk TFAIL "umount $mnt_point"

			tst_res TPASS "mount/umount with \"$mount_opt\" options"
		done
	done
}

tst_run
