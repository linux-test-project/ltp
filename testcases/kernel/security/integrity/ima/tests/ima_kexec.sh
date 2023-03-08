#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Microsoft Corporation
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
# Author: Lachlan Sneff <t-josne@linux.microsoft.com>
#
# Verify that kexec cmdline is measured correctly.
# Test attempts to kexec the existing running kernel image.
# To kexec a different kernel image export IMA_KEXEC_IMAGE=<pathname>.

TST_NEEDS_CMDS="grep kexec sed"
TST_CNT=3
TST_SETUP="setup"

IMA_KEXEC_IMAGE="${IMA_KEXEC_IMAGE:-/boot/vmlinuz-$(uname -r)}"
REQUIRED_POLICY='^measure.*func=KEXEC_CMDLINE'

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
	tst_res TINFO "using kernel $IMA_KEXEC_IMAGE"

	if [ ! -f "$IMA_KEXEC_IMAGE" ]; then
		tst_brk TCONF "kernel image not found, specify path in \$IMA_KEXEC_IMAGE"
	fi

	require_ima_policy_content "$REQUIRED_POLICY"
	policy_readable=1
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
	local res=TFAIL
	local kexec_cmd

	kexec_cmd="$param=$cmdline"
	if [ "$param" = '--reuse-cmdline' ]; then
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
		if [ "$policy_readable" != 1 ]; then
			tst_res TWARN "policy not readable, it might not contain required policy '$REQUIRED_POLICY'"
			res=TBROK
		fi
		tst_brk $res "unable to find a correct measurement"
	fi
	tst_res TPASS "kexec cmdline was measured correctly"
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
