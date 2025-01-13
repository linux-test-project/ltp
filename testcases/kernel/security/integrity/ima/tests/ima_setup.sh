#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018-2025 Petr Vorel <pvorel@suse.cz>
# Author: Mimi Zohar <zohar@linux.ibm.com>

TST_TESTFUNC="test"
TST_SETUP_CALLER="$TST_SETUP"
TST_SETUP="ima_setup"
TST_CLEANUP_CALLER="$TST_CLEANUP"
TST_CLEANUP="ima_cleanup"
TST_NEEDS_ROOT=1
TST_MOUNT_DEVICE=1
TST_SKIP_LSM_WARNINGS=1

# TST_MOUNT_DEVICE can be unset, therefore specify explicitly
TST_NEEDS_TMPDIR=1

SYSFS="/sys"
UMOUNT=
TST_FS_TYPE="ext3"

IMA_FAIL="TFAIL"
IMA_BROK="TBROK"

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

	check_policy_readable || return 1
	grep -q "$pattern" $IMA_POLICY
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
		tst_brk TCONF "test requires builtin IMA $policy policy (e.g. ima_policy=$policy kernel command line parameter)"
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

# Check for required
# 1) IMA builtin policy (based on /proc/cmdline)
# 2) IMA policy content (actual content of /sys/kernel/security/ima/policy)
# When missing CONFIG_IMA_READ_POLICY=y on required policy convert: test, but convert TFAIL => TCONF.
# $REQUIRED_POLICY_CONTENT: file with required IMA policy
# $REQUIRED_BUILTIN_POLICY: IMA policy specified as kernel cmdline
verify_ima_policy()
{
	local check_content line
	local file="$TST_DATAROOT/$REQUIRED_POLICY_CONTENT"

	if [ -z "$REQUIRED_POLICY_CONTENT" -a -z "$REQUIRED_BUILTIN_POLICY" ]; then
		return 0
	fi

	if [ -n "$REQUIRED_POLICY_CONTENT" ]; then
		check_content=1
		if [ -n "$REQUIRED_BUILTIN_POLICY" ] && check_ima_policy_cmdline "$REQUIRED_BUILTIN_POLICY"; then
			tst_res TINFO "booted with IMA policy: $REQUIRED_BUILTIN_POLICY"
			return 0
		fi
	elif [ -n "$REQUIRED_BUILTIN_POLICY" ]; then
		require_ima_policy_cmdline "$REQUIRED_BUILTIN_POLICY"
	fi

	if [ "$check_content" = 1 ]; then
		[ -e $file ] || tst_brk TBROK "policy file '$file' does not exist (LTPROOT=$LTPROOT)"
		tst_res TINFO "test requires IMA policy:"
		cat $file
		if check_policy_readable; then
			# check IMA policy content
			while read line; do
				if ! grep -q "$line" $IMA_POLICY; then
					tst_brk TCONF "missing required policy '$line'"
				fi
				IMA_POLICY_CHECKED=1
			done < $file
		else
			tst_res TINFO "policy is not readable, failure will be treated as TCONF"
			IMA_FAIL="TCONF"
			IMA_BROK="TCONF"
		fi
	fi
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

	verify_ima_policy

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
	ima|ima-buf|ima-ng|ima-sig) DIGEST_INDEX=4 ;;
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

	if [ -z "$DIGEST_INDEX" ]; then
		tst_res TWARN "Cannot find digest index (template: '$template')"
		return 1
	fi
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

	if [ -z "$DIGEST_INDEX" ]; then
		set_digest_index
	fi
	if [ -z "$DIGEST_INDEX" ]; then
		return 1
	fi

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
	local algorithm digest expected_digest line

	# need to read file to get updated $ASCII_MEASUREMENTS
	cat $test_file > /dev/null

	line="$(grep $test_file $ASCII_MEASUREMENTS | tail -1)"

	if get_algorithm_digest "$line" > tmp; then
		algorithm=$(cat tmp | cut -d'|' -f1)
		digest=$(cat tmp | cut -d'|' -f2)
	else
		tst_brk $IMA_BROK "failed to get algorithm/digest for '$test_file'"
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
