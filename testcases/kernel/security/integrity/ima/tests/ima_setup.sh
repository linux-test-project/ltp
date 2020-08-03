#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
# Author: Mimi Zohar <zohar@linux.ibm.com>

TST_TESTFUNC="test"
TST_SETUP_CALLER="$TST_SETUP"
TST_SETUP="ima_setup"
TST_CLEANUP_CALLER="$TST_CLEANUP"
TST_CLEANUP="ima_cleanup"
TST_NEEDS_ROOT=1

# TST_NEEDS_DEVICE can be unset, therefore specify explicitly
TST_NEEDS_TMPDIR=1

. tst_test.sh

SYSFS="/sys"
UMOUNT=
TST_FS_TYPE="ext3"

# TODO: find support for rmd128 rmd256 rmd320 wp256 wp384 tgr128 tgr160
compute_digest()
{
	local algorithm="$1"
	local file="$2"
	local digest

	digest="$(${algorithm}sum $file 2>/dev/null | cut -f1 -d ' ')"
	if [ -n "$digest" ]; then
		echo "$digest"
		return 0
	fi

	digest="$(openssl $algorithm $file 2>/dev/null | cut -f2 -d ' ')"
	if [ -n "$digest" ]; then
		echo "$digest"
		return 0
	fi

	# uncommon ciphers
	local arg="$algorithm"
	case "$algorithm" in
	tgr192) arg="tiger" ;;
	wp512) arg="whirlpool" ;;
	esac

	digest="$(rdigest --$arg $file 2>/dev/null | cut -f1 -d ' ')"
	if [ -n "$digest" ]; then
		echo "$digest"
		return 0
	fi
	return 1
}

check_policy_readable()
{
	if [ ! -f $IMA_POLICY ]; then
		tst_res TINFO "missing $IMA_POLICY (reboot or CONFIG_IMA_WRITE_POLICY=y required)"
		return 1
	fi
	cat $IMA_POLICY > /dev/null 2>/dev/null
}

require_policy_readable()
{
	if [ ! -f $IMA_POLICY ]; then
		tst_brk TCONF "missing $IMA_POLICY (reboot or CONFIG_IMA_WRITE_POLICY=y required)"
	fi
	if ! check_policy_readable; then
		tst_brk TCONF "cannot read IMA policy (CONFIG_IMA_READ_POLICY=y required)"
	fi
}

check_ima_policy_content()
{
	local pattern="$1"
	local grep_params="${2--q}"

	check_policy_readable || return 1
	grep $grep_params "$pattern" $IMA_POLICY
}

require_ima_policy_content()
{
	local pattern="$1"
	local grep_params="${2--q}"

	require_policy_readable
	if ! grep $grep_params "$pattern" $IMA_POLICY; then
		tst_brk TCONF "IMA policy does not specify '$pattern'"
	fi
}

require_ima_policy_cmdline()
{
	local policy="$1"
	local i

	grep -q "ima_$policy" /proc/cmdline && return
	for i in $(cat /proc/cmdline); do
		if echo "$i" | grep -q '^ima_policy='; then
			echo "$i" | grep -q -e "|[ ]*$policy" -e "$policy[ ]*|" -e "=$policy" && return
		fi
	done
	tst_brk TCONF "IMA measurement tests require builtin IMA $policy policy (e.g. ima_policy=$policy kernel parameter)"
}

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

	if [ -r "$config" ]; then
		tst_res TINFO "IMA kernel config:"
		for i in $(grep ^CONFIG_IMA $config); do
			tst_res TINFO "$i"
		done
	fi

	tst_res TINFO "/proc/cmdline: $(cat /proc/cmdline)"
}

ima_setup()
{
	SECURITYFS="$(mount_helper securityfs $SYSFS/kernel/security)"

	IMA_DIR="$SECURITYFS/ima"
	[ -d "$IMA_DIR" ] || tst_brk TCONF "IMA not enabled in kernel"
	ASCII_MEASUREMENTS="$IMA_DIR/ascii_runtime_measurements"
	BINARY_MEASUREMENTS="$IMA_DIR/binary_runtime_measurements"
	IMA_POLICY="$IMA_DIR/policy"

	# hack to support running tests locally from ima/tests directory
	if [ ! -d "$TST_DATAROOT" ]; then
		TST_DATAROOT="$LTPROOT/../datafiles/$TST_ID/"
	fi

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

	[ -n "$TST_CLEANUP_CALLER" ] && $TST_CLEANUP_CALLER

	for dir in $UMOUNT; do
		umount $dir
	done

	if [ "$TST_NEEDS_DEVICE" = 1 ]; then
		cd $TST_TMPDIR
		tst_umount
	fi
}

# loop device is needed to use only for tmpfs
TMPDIR="${TMPDIR:-/tmp}"
if [ "$(df -T $TMPDIR | tail -1 | awk '{print $2}')" != "tmpfs" -a -n "$TST_NEEDS_DEVICE" ]; then
	unset TST_NEEDS_DEVICE
fi
