#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Author: Mimi Zohar <zohar@linux.ibm.com>
#
# Verify the boot and PCR aggregates.

TST_CNT=2
TST_NEEDS_CMDS="awk cut ima_boot_aggregate"

. ima_setup.sh

test1()
{
	tst_res TINFO "verify boot aggregate"

	local zero="0000000000000000000000000000000000000000"
	local tpm_bios="$SECURITYFS/tpm0/binary_bios_measurements"
	local ima_measurements="$ASCII_MEASUREMENTS"
	local boot_aggregate boot_hash line

	# IMA boot aggregate
	read line < $ima_measurements
	boot_hash=$(echo $line | awk '{print $(NF-1)}' | cut -d':' -f2)

	if [ ! -f "$tpm_bios" ]; then
		tst_res TINFO "TPM Hardware Support not enabled in kernel or no TPM chip found"

		if [ "${boot_hash}" = "${zero}" ]; then
			tst_res TPASS "bios boot aggregate is 0"
		else
			tst_res TFAIL "bios boot aggregate is not 0"
		fi
	else
		boot_aggregate=$(ima_boot_aggregate $tpm_bios | grep "boot_aggregate:" | cut -d':' -f2)
		if [ "${boot_hash}" = "${boot_aggregate}" ]; then
			tst_res TPASS "bios aggregate matches IMA boot aggregate"
		else
			tst_res TFAIL "bios aggregate does not match IMA boot aggregate"
		fi
	fi
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
		if [ "${pcr}" = "PCR-10" ]; then
			hash="$(echo $line | cut -d':' -f2 | awk '{ gsub (" ", "", $0); print tolower($0) }')"
			[ "${hash}" = "${aggregate_pcr}" ]
			return $?
		fi
	done < $dev_pcrs
	return 1
}

test2()
{
	tst_res TINFO "verify PCR values"
	tst_check_cmds evmctl

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
