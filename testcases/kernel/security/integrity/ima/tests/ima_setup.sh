#!/bin/sh
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Author: Mimi Zohar, zohar@ibm.vnet.ibm.com

TST_TESTFUNC="test"
TST_SETUP_CALLER="$TST_SETUP"
TST_SETUP="ima_setup"
TST_CLEANUP="${TST_CLEANUP:-ima_cleanup}"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1

. tst_test.sh

SYSFS="/sys"
UMOUNT=
TST_FS_TYPE="ext3"

mount_helper()
{
	local type="$1"
	local default_dir="$2"
	local dir

	dir="$(grep ^$type /proc/mounts | cut -d ' ' -f2 | head -1)"
	[ -n "$dir" ] && { echo "$dir"; return; }

	if ! mkdir -p $default_dir; then
		tst_brk TBROK "failed to create $default_dir"
	fi
	if ! mount -t $type $type $default_dir; then
		tst_brk TBROK "failed to mount $type"
	fi
	UMOUNT="$default_dir $UMOUNT"
	echo $default_dir
}

mount_loop_device()
{
	local ret

	tst_mkfs
	tst_mount
	cd $TST_MNTPOINT
}

print_ima_config()
{
	local config="/boot/config-$(uname -r)"
	local i

	tst_res TINFO "/proc/cmdline: $(cat /proc/cmdline)"

	if [ -r "$config" ]; then
		tst_res TINFO "IMA kernel config:"
		for i in $(grep ^CONFIG_IMA $config); do
			tst_res TINFO "$i"
		done
	fi
}

ima_setup()
{
	SECURITYFS="$(mount_helper securityfs $SYSFS/kernel/security)"

	IMA_DIR="$SECURITYFS/ima"
	[ -d "$IMA_DIR" ] || tst_brk TCONF "IMA not enabled in kernel"
	ASCII_MEASUREMENTS="$IMA_DIR/ascii_runtime_measurements"
	BINARY_MEASUREMENTS="$IMA_DIR/binary_runtime_measurements"

	print_ima_config

	if [ "$TST_NEEDS_DEVICE" = 1 ]; then
		tst_res TINFO "\$TMPDIR is on tmpfs => run on loop device"
		mount_loop_device
	fi

	[ -n "$TST_SETUP_CALLER" ] && $TST_SETUP_CALLER
}

ima_cleanup()
{
	local dir
	for dir in $UMOUNT; do
		umount $dir
	done

	if [ "$TST_NEEDS_DEVICE" = 1 ]; then
		cd $TST_TMPDIR
		tst_umount $TST_DEVICE
	fi
}

# loop device is needed to use only for tmpfs
TMPDIR="${TMPDIR:-/tmp}"
if [ "$(df -T $TMPDIR | tail -1 | awk '{print $2}')" != "tmpfs" -a -n "$TST_NEEDS_DEVICE" ]; then
	unset TST_NEEDS_DEVICE
fi
