#!/bin/sh
#
# Copyright (c) International Business Machines  Corp., 2003
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Written by Prakash Narayana (prakashn@us.ibm.com)
# and Michael Reed (mreed10@us.ibm.com)
#
# A script that will test isofs on Linux system.
# It makes ISO9660 file system with different options and also
# mounts the ISO9660 file system with different mount options.
#

TCID=isofs
TST_TOTAL=77
. test.sh

NO_CLEANUP=""

usage()
{
	echo "USAGE: $0 <optional> -n -h"
	exit
}

cleanup()
{
	if [ "$NO_CLEANUP" = "no" ]; then
		tst_resm TINFO "Temporary directory $PWD was not removed"
	else
		tst_rmdir
	fi
}

max_depth=3
max_dirs=4

gen_fs_tree()
{
	local cur_path="$1"
	local cur_depth="$2"

	if [ "$cur_depth" -gt "$max_depth" ]; then
		return
	fi

	for i in $(seq 1 $max_dirs); do
		local new_path="$cur_path/subdir_$i"

		mkdir -p "$new_path"

		dd if=/dev/urandom of="$new_path/file" bs=1024 count=100 >/dev/null 2>&1

		gen_fs_tree "$new_path" $((cur_depth + 1))
	done
}

while getopts :hnd: arg; do
	case $arg in
	h)
		echo ""
		echo "n - The directories created will not be removed"
		echo "h - Help options"
		echo ""
		usage
		echo ""
		;;
	n)
		NO_CLEANUP="no"
		;;
	esac
done

tst_require_root

tst_tmpdir
TST_CLEANUP=cleanup

MNT_POINT="$PWD/mnt"
MAKE_FILE_SYS_DIR="$PWD/files"

mkdir -p -m 777 $MNT_POINT
mkdir -p $MAKE_FILE_SYS_DIR

# Generated directories and files
mkdir -p $MAKE_FILE_SYS_DIR
gen_fs_tree "$MAKE_FILE_SYS_DIR" 1

# Make ISO9660 file system with different options.
# Mount the ISO9660 file system with different mount options.

tst_check_cmds mkisofs

for mkisofs_opt in \
	" " \
	"-J" \
	"-hfs -D" \
	" -R " \
	"-R -J" \
	"-f -l -D -J -L -R" \
	"-allow-lowercase -allow-multidot -iso-level 3 -f -l -D -J -L -R"
do
	rm -f isofs.iso
	mkisofs -o isofs.iso -quiet $mkisofs_opt $MAKE_FILE_SYS_DIR 2> /dev/null
	if [ $? -eq 0 ]; then
		tst_resm TPASS \
			"mkisofs -o isofs.iso -quiet $mkisofs_opt $MAKE_FILE_SYS_DIR"
	else
		tst_resm TFAIL \
			tst_resm TFAIL "mkisofs -o isofs.iso -quiet $mkisofs_opt $MAKE_FILE_SYS_DIR"
		continue
	fi

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
		# "loop,sbsector=32"
	do
		mount -t iso9660 -o $mount_opt isofs.iso $MNT_POINT
		if [ $? -ne 0 ]; then
			tst_resm TFAIL \
				"mount -t iso9660 -o $mount_opt isofs.iso $MNT_POINT"
			continue
		fi

		ls -lR $MNT_POINT > /dev/null
		if [ $? -ne 0 ]; then
			tst_resm TFAIL "ls -lR $MNT_POINT"
		fi

		umount $MNT_POINT
		if [ $? -ne 0 ]; then
			tst_brkm TFAIL "umount $MNT_POINT"
		fi

		tst_resm TPASS "mount/umount with \"$mount_opt\" options"
	done
done

tst_exit
