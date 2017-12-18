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
#
# Verify the boot and PCR aggregates.

TST_NEEDS_CMDS="ima_boot_aggregate ima_measure"
TST_CNT=3

. ima_setup.sh

test1()
{
	tst_res TINFO "verify boot aggregate"

	local zero="0000000000000000000000000000000000000000"
	local tpm_bios="$SECURITYFS/tpm0/binary_bios_measurements"
	local ima_measurements="$ASCII_MEASUREMENTS"
	local boot_aggregate boot_hash ima_hash line

	# IMA boot aggregate
	read line < $ima_measurements
	ima_hash=$(expr substr "${line}" 49 40)

	if [ ! -f "$tpm_bios" ]; then
		tst_res TINFO "TPM not builtin kernel, or TPM not enabled"

		if [ "${ima_hash}" = "${zero}" ]; then
			tst_res TPASS "bios boot aggregate is 0"
		else
			tst_res TFAIL "bios boot aggregate is not 0"
		fi
	else
		boot_aggregate=$(ima_boot_aggregate $tpm_bios)
		boot_hash=$(expr substr $boot_aggregate 16 40)
		if [ "${ima_hash}" = "${boot_hash}" ]; then
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

	local ima_measurements="$BINARY_MEASUREMENTS"
	local aggregate_pcr="$(ima_measure $ima_measurements --validate)"
	local dev_pcrs="$1"
	local ret=0

	while read line; do
		pcr=$(expr substr "${line}" 1 6)
		if [ "${pcr}" = "PCR-10" ]; then
			aggr=$(expr substr "${aggregate_pcr}" 26 59)
			pcr=$(expr substr "${line}" 9 59)
			[ "${pcr}" = "${aggr}" ] || ret=$?
		fi
	done < $dev_pcrs
	return $ret
}

test2()
{
	tst_res TINFO "verify PCR values"

	# Would be nice to know where the PCRs are located. Is this safe?
	local pcrs_path="$(find $SYSFS/devices/ | grep pcrs)"
	if [ $? -eq 0 ]; then
		validate_pcr $pcrs_path
		if [ $? -eq 0 ]; then
			tst_res TPASS "aggregate PCR value matches real PCR value"
		else
			tst_res TFAIL "aggregate PCR value does not match real PCR value"
		fi
	else
		tst_res TCONF "TPM not enabled, no PCR value to validate"
	fi
}

test3()
{
	tst_res TINFO "verify template hash value"

	local ima_measurements="$BINARY_MEASUREMENTS"
	ima_measure $ima_measurements --verify --validate
	if [ $? -eq 0 ]; then
		tst_res TPASS "verified IMA template hash values"
	else
		tst_res TFAIL "error verifing IMA template hash values"
	fi
}

tst_run
