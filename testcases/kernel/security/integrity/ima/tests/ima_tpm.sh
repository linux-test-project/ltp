#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
# Author: Mimi Zohar <zohar@linux.ibm.com>
#
# Verify the boot and PCR aggregates.

TST_CNT=2
TST_NEEDS_CMDS="awk cut tail"
TST_SETUP="setup"

EVMCTL_REQUIRED='1.3.1'
ERRMSG_EVMCTL="=> install evmctl >= $EVMCTL_REQUIRED"
ERRMSG_TPM="TPM hardware support not enabled in kernel or no TPM chip found"

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
		tst_res TINFO "$ERRMSG_TPM, testing TPM-bypass"
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

read_pcr_tpm1()
{
	local pcrs_path="/sys/class/tpm/tpm0/device/pcrs"
	local evmctl_required="1.1"
	local hash pcr

	if [ ! -f "$pcrs_path" ]; then
		pcrs_path="/sys/class/misc/tpm0/device/pcrs"
	elif ! check_evmctl $evmctl_required; then
		echo "evmctl >= $evmctl_required required"
		return 32
	fi

	if [ ! -f "$pcrs_path" ]; then
		echo "missing PCR file $pcrs_path ($ERRMSG_TPM)"
		return 32
	fi

	while read line; do
		pcr="$(echo $line | cut -d':' -f1)"
		hash="$(echo $line | cut -d':' -f2 | awk '{ gsub (" ", "", $0); print tolower($0) }')"
		echo "$pcr: $hash"
	done < $pcrs_path

	return 0
}

# NOTE: TPM 1.2 would require to use tss1pcrread which is not fully adopted
# by distros yet.
read_pcr_tpm2()
{
	local pcrmax=23
	local pcrread="tsspcrread -halg $ALGORITHM"
	local i pcr

	if ! tst_cmd_available tsspcrread; then
		echo "tsspcrread not found"
		return 32
	fi

	for i in $(seq 0 $pcrmax); do
		pcr=$($pcrread -ha "$i" -ns)
		if [ $? -ne 0 ]; then
			echo "tsspcrread failed: $pcr"
			return 1
		fi
		printf "PCR-%02d: %s\n" $i "$pcr"
	done

	return 0
}

get_pcr10_aggregate()
{
	local cmd="evmctl -vv ima_measurement $BINARY_MEASUREMENTS"
	local msg="$ERRMSG_EVMCTL"
	local res=TCONF
	local pcr ret

	if [ -z "$MISSING_EVMCTL" ]; then
		msg=
		res=TFAIL
	fi

	$cmd > hash.txt 2>&1
	ret=$?
	if [ $ret -ne 0 -a -z "$MISSING_EVMCTL" ]; then
		tst_res TFAIL "evmctl failed, trying with --ignore-violations"
		cmd="$cmd --ignore-violations"
		$cmd > hash.txt 2>&1
		ret=$?
	elif [ $ret -ne 0 -a "$MISSING_EVMCTL" = 1 ]; then
		tst_res TFAIL "evmctl failed $msg"
		return
	fi

	[ $ret -ne 0 ] && tst_res TWARN "evmctl failed, trying to continue $msg"

	pcr=$(grep -E "^($ALGORITHM: )*PCRAgg(.*10)*:" hash.txt | tail -1 \
		| awk '{print $NF}')

	if [ -z "$pcr" ]; then
		tst_res $res "failed to find aggregate PCR-10 $msg"
		tst_res TINFO "hash file:"
		cat hash.txt >&2
		return
	fi

	echo "$pcr"
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

	if [ -z "$TPM_VERSION" ]; then
		tst_res TWARN "TPM-bypass failed, trying to test TPM device (unknown TPM version)"
		MAYBE_TPM2=1
	fi

	if [ "$MISSING_EVMCTL" = 1 ]; then
		if [ ! -f "$tpm_bios" ]; then
			tst_res TCONF "missing $tpm_bios $ERRMSG_EVMCTL"
			return
		fi
		tst_check_cmds ima_boot_aggregate || return

		cmd="ima_boot_aggregate -f $tpm_bios"

		# TCONF: libcrypto and openssl development packages required
		$cmd
		if [ $? -eq 32 ]; then
			tst_res TCONF "$cmd returned TCONF"
			return
		fi
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

test2()
{
	local hash pcr_aggregate out ret

	tst_res TINFO "verify PCR values"

	if [ "$MAYBE_TPM2" = 1 ]; then
		tst_res TINFO "TPM version not detected ($ERRMSG_TPM), assume TPM 2"
		TPM_VERSION=2
	fi

	if [ -z "$TPM_VERSION" ]; then
		tst_brk TCONF "TPM version not detected ($ERRMSG_TPM)"
	fi

	if [ "$ALGORITHM" = "sha1" -a "$MISSING_EVMCTL" = 1 ]; then
		tst_check_cmds evmctl || return 1
	fi

	out=$(read_pcr_tpm$TPM_VERSION)
	ret=$?

	if [ $ret -ne 0 ]; then
		case "$ret" in
			1) tst_res TFAIL "$out";;
			32) tst_res TCONF "$out";;
			*) tst_brk TBROK "unsupported return type '$1'";;
		esac
		return
	fi

	hash=$(echo "$out" | grep "^PCR-10" | cut -d' ' -f2)

	if [ -z "$out" ]; then
		tst_res TFAIL "PCR-10 hash not found"
		return
	fi

	tst_res TINFO "real PCR-10: '$hash'"

	get_pcr10_aggregate > tmp.txt
	pcr_aggregate="$(cat tmp.txt)"
	if [ -z "$pcr_aggregate" ]; then
		return
	fi
	tst_res TINFO "aggregate PCR-10: '$pcr_aggregate'"

	if [ "$hash" = "$pcr_aggregate" ]; then
		tst_res TPASS "aggregate PCR value matches real PCR value"
	else
		tst_res TFAIL "aggregate PCR value does not match real PCR value"
	fi
}

. ima_setup.sh
tst_run
