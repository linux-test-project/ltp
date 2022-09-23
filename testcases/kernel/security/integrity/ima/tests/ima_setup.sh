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
TST_MOUNT_DEVICE=1

# TST_MOUNT_DEVICE can be unset, therefore specify explicitly
TST_NEEDS_TMPDIR=1

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

require_policy_writable()
{
	local err="IMA policy already loaded and kernel not configured to enable multiple writes to it (need CONFIG_IMA_WRITE_POLICY=y)"

	[ -f $IMA_POLICY ] || tst_brk TCONF "$err"
	# CONFIG_IMA_READ_POLICY
	echo "" 2> log > $IMA_POLICY
	grep -q "Device or resource busy" log && tst_brk TCONF "$err"
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

check_ima_policy_cmdline()
{
	local policy="$1"
	local i

	grep -q "ima_$policy" /proc/cmdline && return
	for i in $(cat /proc/cmdline); do
		if echo "$i" | grep -q '^ima_policy='; then
			echo "$i" | grep -q -e "|[ ]*$policy" -e "$policy[ ]*|" -e "=$policy" && return 0
		fi
	done
	return 1
}

require_ima_policy_cmdline()
{
	local policy="$1"

	check_ima_policy_cmdline $policy || \
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

print_ima_config()
{
	local config="${KCONFIG_PATH:-/boot/config-$(uname -r)}"
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

	if [ "$TST_MOUNT_DEVICE" = 1 ]; then
		tst_res TINFO "\$TMPDIR is on tmpfs => run on loop device"
		cd "$TST_MNTPOINT"
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
}

set_digest_index()
{
	DIGEST_INDEX=

	local template="$(tail -1 $ASCII_MEASUREMENTS | cut -d' ' -f 3)"
	local i word

	# parse digest index
	# https://www.kernel.org/doc/html/latest/security/IMA-templates.html#use
	case "$template" in
	ima|ima-ng|ima-sig) DIGEST_INDEX=4 ;;
	*)
		# using ima_template_fmt kernel parameter
		local IFS="|"
		i=4
		for word in $template; do
			if [ "$word" = 'd' -o "$word" = 'd-ng' ]; then
				DIGEST_INDEX=$i
				break
			fi
			i=$((i+1))
		done
	esac

	[ -z "$DIGEST_INDEX" ] && tst_brk TCONF \
		"Cannot find digest index (template: '$template')"
}

get_algorithm_digest()
{
	local line="$1"
	local delimiter=':'
	local algorithm digest

	if [ -z "$line" ]; then
		echo "measurement record not found"
		return 1
	fi

	[ -z "$DIGEST_INDEX" ] && set_digest_index
	digest=$(echo "$line" | cut -d' ' -f $DIGEST_INDEX)
	if [ -z "$digest" ]; then
		echo "digest not found (index: $DIGEST_INDEX, line: '$line')"
		return 1
	fi

	if [ "${digest#*$delimiter}" != "$digest" ]; then
		algorithm=$(echo "$digest" | cut -d $delimiter -f 1)
		digest=$(echo "$digest" | cut -d $delimiter -f 2)
	else
		case "${#digest}" in
		32) algorithm="md5" ;;
		40) algorithm="sha1" ;;
		*)
			echo "algorithm must be either md5 or sha1 (digest: '$digest')"
			return 1 ;;
		esac
	fi
	if [ -z "$algorithm" ]; then
		echo "algorithm not found"
		return 1
	fi
	if [ -z "$digest" ]; then
		echo "digest not found"
		return 1
	fi

	echo "$algorithm|$digest"
}

ima_check()
{
	local test_file="$1"
	local algorithm digest expected_digest line tmp

	# need to read file to get updated $ASCII_MEASUREMENTS
	cat $test_file > /dev/null

	line="$(grep $test_file $ASCII_MEASUREMENTS | tail -1)"

	if tmp=$(get_algorithm_digest "$line"); then
		algorithm=$(echo "$tmp" | cut -d'|' -f1)
		digest=$(echo "$tmp" | cut -d'|' -f2)
	else
		tst_res TBROK "failed to get algorithm/digest for '$test_file': $tmp"
	fi

	tst_res TINFO "computing digest for $algorithm algorithm"
	expected_digest="$(compute_digest $algorithm $test_file)" || \
		tst_brk TCONF "cannot compute digest for $algorithm algorithm"

	if [ "$digest" = "$expected_digest" ]; then
		tst_res TPASS "correct digest found"
	else
		tst_res TFAIL "digest not found"
	fi
}

# check_evmctl REQUIRED_TPM_VERSION
# return: 0: evmctl is new enough, 1: version older than required (or version < v0.9)
check_evmctl()
{
	local required="$1"

	local r1="$(echo $required | cut -d. -f1)"
	local r2="$(echo $required | cut -d. -f2)"
	local r3="$(echo $required | cut -d. -f3)"
	[ -z "$r3" ] && r3=0

	tst_is_int "$r1" || tst_brk TBROK "required major version not int ($v1)"
	tst_is_int "$r2" || tst_brk TBROK "required minor version not int ($v2)"
	tst_is_int "$r3" || tst_brk TBROK "required patch version not int ($v3)"

	tst_check_cmds evmctl || return 1

	local v="$(evmctl --version | cut -d' ' -f2)"
	[ -z "$v" ] && return 1
	tst_res TINFO "evmctl version: $v"

	local v1="$(echo $v | cut -d. -f1)"
	local v2="$(echo $v | cut -d. -f2)"
	local v3="$(echo $v | cut -d. -f3)"
	[ -z "$v3" ] && v3=0

	if [ $v1 -lt $r1 ] || [ $v1 -eq $r1 -a $v2 -lt $r2 ] || \
		[ $v1 -eq $r1 -a $v2 -eq $r2 -a $v3 -lt $r3 ]; then
		return 1
	fi
	return 0
}

# require_evmctl REQUIRED_TPM_VERSION
require_evmctl()
{
	local required="$1"

	if ! check_evmctl $required; then
		tst_brk TCONF "evmctl >= $required required"
	fi
}

# loop device is needed to use only for tmpfs
TMPDIR="${TMPDIR:-/tmp}"
if tst_supported_fs -d $TMPDIR -s "tmpfs"; then
	unset TST_MOUNT_DEVICE
fi

. tst_test.sh
