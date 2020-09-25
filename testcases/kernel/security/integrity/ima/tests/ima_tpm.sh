#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
# Author: Mimi Zohar <zohar@linux.ibm.com>
#
# Verify the boot and PCR aggregates.

TST_CNT=2
TST_NEEDS_CMDS="awk cut"
TST_SETUP="setup"

. ima_setup.sh

EVMCTL_REQUIRED='1.3.1'
ERRMSG_EVMCTL="=> install evmctl >= $EVMCTL_REQUIRED"

setup()
{
	local config="${KCONFIG_PATH:-/boot/config-$(uname -r)}"
	local line tmp

	read line < $ASCII_MEASUREMENTS
	if tmp=$(get_algorithm_digest "$line"); then
		ALGORITHM=$(echo "$tmp" | cut -d'|' -f1)
		DIGEST=$(echo "$tmp" | cut -d'|' -f2)
	else
		tst_brk TBROK "failed to get algorithm/digest: $tmp"
	fi
	tst_res TINFO "used algorithm: $ALGORITHM"

	TPM_VERSION="$(get_tpm_version)"
	if [ -z "$TPM_VERSION" ]; then
		tst_res TINFO "TPM hardware support not enabled in kernel or no TPM chip found, testing TPM-bypass"
	else
		tst_res TINFO "TPM major version: $TPM_VERSION"
	fi

	if ! check_evmctl $EVMCTL_REQUIRED; then
		if [ "$ALGORITHM" != "sha1" ]; then
			tst_brk TCONF "algorithm not sha1 ($ALGORITHM) $ERRMSG_EVMCTL"
		fi
		MISSING_EVMCTL=1
	fi

	if [ -r "$config" ]; then
		tst_res TINFO "TPM kernel config:"
		for i in $(grep -e ^CONFIG_.*_TPM -e ^CONFIG_TCG $config); do
			tst_res TINFO "$i"
		done
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

# prints major version: 1: TPM 1.2, 2: TPM 2.0
# or nothing on TPM-bypass (no TPM device)
# WARNING: Detecting TPM 2.0 can fail due kernel not exporting TPM 2.0 files.
get_tpm_version()
{
	if [ -f /sys/class/tpm/tpm0/tpm_version_major ]; then
		cat /sys/class/tpm/tpm0/tpm_version_major
		return
	fi

	if [ -f /sys/class/tpm/tpm0/device/caps -o \
		-f /sys/class/misc/tpm0/device/caps ]; then
		echo 1
		return
	fi

	if [ -c /dev/tpmrm0 -a -c /dev/tpm0 ]; then
		echo 2
		return
	fi

	if [ ! -d /sys/class/tpm/tpm0/ -a ! -d /sys/class/misc/tpm0/ ]; then
		return
	fi

	tst_require_cmds dmesg
	if dmesg | grep -q 'activating TPM-bypass'; then
		return
	elif dmesg | grep -q '1\.2 TPM (device-id'; then
		echo 1
		return
	elif dmesg | grep -q '2\.0 TPM (device-id'; then
		echo 2
		return
	fi
}

test1_tpm_bypass_mode()
{
	local zero=$(echo $DIGEST | awk '{gsub(/./, "0")}; {print}')

	if [ "$DIGEST" = "$zero" ]; then
		tst_res TPASS "bios boot aggregate is $zero"
	else
		tst_res TFAIL "bios boot aggregate is not $zero ($DIGEST), kernel didn't export TPM 2.0 files for TPM device?"
		return 1
	fi
}

test1_hw_tpm()
{
	local tpm_bios="$SECURITYFS/tpm0/binary_bios_measurements"
	local cmd="evmctl ima_boot_aggregate -v"
	local boot_aggregate

	[ -z "$TPM_VERSION" ] && \
		tst_res TWARN "TPM-bypass failed, trying to test TPM device (unknown TPM version)"

	if [ "$MISSING_EVMCTL" = 1 ]; then
		if [ ! -f "$tpm_bios" ]; then
			tst_res TCONF "missing $tpm_bios $ERRMSG_EVMCTL"
			return
		fi
		tst_check_cmds ima_boot_aggregate || return
		cmd="ima_boot_aggregate -f $tpm_bios"
	fi
	tst_res TINFO "using command: $cmd"

	boot_aggregate=$($cmd | grep "$ALGORITHM:" | cut -d':' -f2)
	if [ -z "$boot_aggregate" ]; then
		tst_res TFAIL "failed to get boot aggregate"
		return
	fi
	tst_res TINFO "IMA boot aggregate: '$boot_aggregate'"

	if [ "$DIGEST" = "$boot_aggregate" ]; then
		tst_res TPASS "bios boot aggregate matches IMA boot aggregate"
	else
		tst_res TFAIL "bios boot aggregate does not match IMA boot aggregate ($DIGEST)"
	fi
}

test1()
{
	tst_res TINFO "verify boot aggregate"

	# deliberately try test1_hw_tpm() if test1_tpm_bypass_mode() fails
	[ -z "$TPM_VERSION" ] && test1_tpm_bypass_mode || test1_hw_tpm
}

# Probably cleaner to programmatically read the PCR values directly
# from the TPM, but that would require a TPM library. For now, use
# the PCR values from /sys/devices.
validate_pcr()
{
	tst_res TINFO "verify PCR (Process Control Register)"

	local dev_pcrs="$1"
	local pcr hash aggregate_pcr

	aggregate_pcr="$(evmctl -v ima_measurement $BINARY_MEASUREMENTS 2>&1 | \
		grep 'HW PCR-10:' | awk '{print $3}')"
	if [ -z "$aggregate_pcr" ]; then
		tst_res TFAIL "failed to get PCR-10"
		return 1
	fi

	while read line; do
		pcr="$(echo $line | cut -d':' -f1)"
		if [ "$pcr" = "PCR-10" ]; then
			hash="$(echo $line | cut -d':' -f2 | awk '{ gsub (" ", "", $0); print tolower($0) }')"
			[ "$hash" = "$aggregate_pcr" ]
			return $?
		fi
	done < $dev_pcrs
	return 1
}

test2()
{
	tst_res TINFO "verify PCR values"
	tst_check_cmds evmctl || return

	tst_res TINFO "evmctl version: $(evmctl --version)"

	local pcrs_path="/sys/class/tpm/tpm0/device/pcrs"
	if [ -f "$pcrs_path" ]; then
		tst_res TINFO "new PCRS path, evmctl >= 1.1 required"
	else
		pcrs_path="/sys/class/misc/tpm0/device/pcrs"
	fi

	if [ -f "$pcrs_path" ]; then
		validate_pcr $pcrs_path
		if [ $? -eq 0 ]; then
			tst_res TPASS "aggregate PCR value matches real PCR value"
		else
			tst_res TFAIL "aggregate PCR value does not match real PCR value"
		fi
	else
		tst_res TCONF "TPM Hardware Support not enabled in kernel or no TPM chip found"
	fi
}

tst_run
