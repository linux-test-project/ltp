#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Microsoft Corporation
# Copyright (c) 2020-2025 Petr Vorel <pvorel@suse.cz>
# Author: Lachlan Sneff <t-josne@linux.microsoft.com>
#
# Verify that kexec cmdline is measured correctly.
# Test attempts to kexec the existing running kernel image.
#
# To kexec a different kernel image export IMA_KEXEC_IMAGE=<pathname>.
# Test requires example IMA policy loadable with LTP_IMA_LOAD_POLICY=1.
#
# Test requires CONFIG_HAVE_IMA_KEXEC=y (CONFIG_IMA_KEXEC is not mandatory).

TST_NEEDS_CMDS="grep kexec sed"
TST_CNT=3
TST_SETUP="setup"
TST_MIN_KVER="5.3"

REQUIRED_POLICY_CONTENT='kexec.policy'

measure()
{
	local cmdline="$1"
	local algorithm digest expected_digest found

	printf "$cmdline" > file1
	grep "kexec-cmdline" $ASCII_MEASUREMENTS > file2

	while read found
	do
		algorithm=$(echo "$found" | cut -d' ' -f4 | cut -d':' -f1)
		digest=$(echo "$found" | cut -d' ' -f4 | cut -d':' -f2)

		expected_digest=$(compute_digest $algorithm file1)

		if [ "$digest" = "$expected_digest" ]; then
			return 0
		fi
	done < file2

	return 1
}

setup()
{
	local arch f uname

	# detect kernel from BOOT_IMAGE in /proc/cmdline
	if [ ! -f "$IMA_KEXEC_IMAGE" ] && grep -q '^BOOT_IMAGE' /proc/cmdline; then
		for arg in $(cat /proc/cmdline); do
			if echo "$arg" |grep -q '^BOOT_IMAGE'; then
				eval "$arg"
			fi
		done

		tst_res TINFO "using as kernel BOOT_IMAGE from /proc/cmdline: '$BOOT_IMAGE'"

		# replace grub partition, e.g. (hd0,gpt2) => /boot
		if echo "$BOOT_IMAGE" |grep -q '(.d[0-9]'; then
			echo "$BOOT_IMAGE" | sed 's|(.*,.*)/|/boot/|'
		fi

		if [ -f "$BOOT_IMAGE" ]; then
			IMA_KEXEC_IMAGE="$BOOT_IMAGE"
		fi
	fi

	# detect kernel in /boot
	if [ ! -f "$IMA_KEXEC_IMAGE" ]; then
		uname="$(uname -r)"

		for f in \
			/boot/vmlinuz-$uname \
			/boot/vmlinux-$uname \
			/boot/Image-$uname \
			/boot/image-$uname \
		; do
			if [ -f "$f" ]; then
				break
			fi
		done

		# aarch64 often uses compression
		if [ ! -f "$f" ]; then
			f="$(ls /boot/Image-$uname.* || true)"
		fi

		if [ -f "$f" ]; then
			IMA_KEXEC_IMAGE="$f"
		fi
	fi

	if [ ! -f "$IMA_KEXEC_IMAGE" ]; then
		tst_brk TCONF "kernel image not found, specify path in \$IMA_KEXEC_IMAGE"
	fi

	tst_res TINFO "using kernel $IMA_KEXEC_IMAGE"

	tst_res TINFO "$(kexec -v)"

	REUSE_CMDLINE_SUPPORTED=
	if kexec -h 2>&1 | grep -q reuse-cmdline; then
		REUSE_CMDLINE_SUPPORTED=1
	fi
}

kexec_failure_hint()
{
	local sb_enabled

	if tst_cmd_available bootctl; then
		if bootctl status 2>/dev/null | grep -qi 'Secure Boot: enabled'; then
			sb_enabled=1
		fi
	elif tst_cmd_available dmesg; then
		if dmesg | grep -qi 'Secure boot enabled'; then
			sb_enabled=1
		fi
	fi
	if [ "$sb_enabled" ]; then
		tst_res TWARN "secure boot is enabled, kernel image may not be signed"
	fi

	if check_ima_policy_content '^appraise.*func=KEXEC_KERNEL_CHECK'; then
		tst_res TWARN "'func=KEXEC_KERNEL_CHECK' appraise policy loaded, kernel image may not be signed"
	fi
}

kexec_test()
{
	local param="$1"
	local cmdline="$2"
	local kexec_cmd

	kexec_cmd="$param=$cmdline"
	if [ "$param" = '--reuse-cmdline' ]; then
		if [ "$REUSE_CMDLINE_SUPPORTED" != 1 ]; then
			tst_res TCONF "--reuse-cmdline not supported"
			return
		fi
		cmdline="$(sed 's/BOOT_IMAGE=[^ ]* //' /proc/cmdline)"
		kexec_cmd="$param"
	fi

	kexec_cmd="kexec -s -l $IMA_KEXEC_IMAGE $kexec_cmd"
	tst_res TINFO "testing $kexec_cmd"
	if ! $kexec_cmd 2>err; then
		kexec_failure_hint
		tst_brk TBROK "kexec failed: $(cat err)"
	fi

	ROD kexec -su
	if ! measure "$cmdline"; then
		tst_res $IMA_FAIL "unable to find a correct measurement"
	else
		tst_res TPASS "kexec cmdline was measured correctly"
	fi
}

test()
{
	case $1 in
	1) kexec_test '--reuse-cmdline';;
	2) kexec_test '--append' 'foo';;
	3) kexec_test '--command-line' 'bar';;
	esac
}

. ima_setup.sh
tst_run
