#!/bin/sh

################################################################################
##                                                                            ##
## Copyright (C) 2009 IBM Corporation                                         ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
################################################################################
#
# File :        ima_tpm.sh
#
# Description:  This file verifies the boot and PCR aggregates
#
# Author:       Mimi Zohar, zohar@ibm.vnet.ibm.com
#
# Return        - zero on success
#               - non zero on failure. return value from commands ($RC)
################################################################################
export TST_TOTAL=3
export TCID="ima_tpm"

init()
{
	tst_check_cmds ima_boot_aggregate ima_measure
}

# Function:     test01
# Description   - Verify boot aggregate value is correct
test01()
{
	zero="0000000000000000000000000000000000000000"

	# IMA boot aggregate
	ima_measurements=$SECURITYFS/ima/ascii_runtime_measurements
	read line < $ima_measurements
	ima_aggr=$(expr substr "${line}" 49 40)

	# verify TPM is available and enabled.
	tpm_bios=$SECURITYFS/tpm0/binary_bios_measurements
	if [ ! -f "$tpm_bios" ]; then
		tst_brkm TCONF "TPM not builtin kernel, or TPM not enabled"

		if [ "${ima_aggr}" = "${zero}" ]; then
			tst_resm TPASS "bios boot aggregate is 0."
		else
			tst_resm TFAIL "bios boot aggregate is not 0."
		fi
	else
		boot_aggregate=$(ima_boot_aggregate $tpm_bios)
		boot_aggr=$(expr substr $boot_aggregate 16 40)
		if [ "x${ima_aggr}" = "x${boot_aggr}" ]; then
			tst_resm TPASS "bios aggregate matches IMA boot aggregate."
		else
			tst_resm TFAIL "bios aggregate does not match IMA boot aggregate."
		fi
	fi
}

# Probably cleaner to programmatically read the PCR values directly
# from the TPM, but that would require a TPM library. For now, use
# the PCR values from /sys/devices.
validate_pcr()
{
	ima_measurements=$SECURITYFS/ima/binary_runtime_measurements
	aggregate_pcr=$(ima_measure $ima_measurements --validate)
	dev_pcrs=$1
	RC=0

	while read line ; do
		pcr=$(expr substr "${line}" 1 6)
		if [ "${pcr}" = "PCR-10" ]; then
			aggr=$(expr substr "${aggregate_pcr}" 26 59)
			pcr=$(expr substr "${line}" 9 59)
			[ "${pcr}" = "${aggr}" ] || RC=$?
		fi
	done < $dev_pcrs
	return $RC
}

# Function:     test02
# Description	- Verify ima calculated aggregate PCR values matches
#		  actual PCR value.
test02()
{

	# Would be nice to know where the PCRs are located.  Is this safe?
	PCRS_PATH=$(find /$SYSFS/devices/ | grep pcrs)
	if [ $? -eq 0 ]; then
		validate_pcr $PCRS_PATH
		if [ $? -eq 0 ]; then
			tst_resm TPASS "aggregate PCR value matches real PCR value."
		else
			tst_resm TFAIL "aggregate PCR value does not match real PCR value."
		fi
	else
		tst_resm TFAIL "TPM not enabled, no PCR value to validate"
	fi
}

# Function:     test03
# Description 	- Verify template hash value for IMA entry is correct.
test03()
{

	ima_measurements=$SECURITYFS/ima/binary_runtime_measurements
	aggregate_pcr=$(ima_measure $ima_measurements --verify --validate) > /dev/null
	if [ $? -eq 0 ]; then
		tst_resm TPASS "verified IMA template hash values."
	else
		tst_resm TFAIL "error verifing IMA template hash values."
	fi
}

. ima_setup.sh

setup
TST_CLEANUP=cleanup

init
test01
test02
test03

tst_exit
