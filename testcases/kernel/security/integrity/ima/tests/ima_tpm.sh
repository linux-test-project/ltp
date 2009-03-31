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
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
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
init()
{
	export TST_TOTAL=3
	export TCID="init"
	export TST_COUNT=0
	RC=0

	# verify ima_boot_aggregate is available
	which ima_boot_aggregate &> /dev/null || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TINFO $LTPTMP/imalog.$$\
		 "$TCID: ima_tpm.sh test requires openssl-devel, skipping"
		return $RC
	fi

	# verify ima_measure is available
	which ima_measure &> /dev/null || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TINFO $LTPTMP/imalog.$$\
		 "$TCID: ima_tpm.sh test requires openssl-devel, skipping"
	fi
	return $RC
}

# Function:     test01
# Description   - Verify boot aggregate value is correct
test01()
{
	TCID="test01"
	TST_COUNT=1
	RC=0

	# IMA boot aggregate
	ima_measurements=$SECURITYFS/ima/ascii_runtime_measurements
	read line < $ima_measurements

	# verify TPM is available and enabled.
	tpm_bios=$SECURITYFS/tpm0/binary_bios_measurements
	if [ ! -f $tpm_bios ]; then
		tst_res TINFO $LTPTMP/imalog.$$\
		 "$TCID: no TPM, TPM not builtin kernel, or TPM not enabled"

		[ "${line:49:40}" -eq 0 ] || RC=$?
		if [ $RC -eq 0 ]; then
			tst_res TPASS $LTPTMP/imalog.$$\
			 "$TCID: bios boot aggregate is 0."
		else
			tst_res TFAIL $LTPTMP/imalog.$$\
			 "$TCID: bios boot aggregate is not 0."
		fi
	else
		boot_aggregate=`ima_boot_aggregate $tpm_bios`

		[ "${line:48:40}" == "${boot_aggregate:15:40}" ] ||  RC=$?
		if [ $RC -eq 0 ]; then
			tst_res TPASS $LTPTMP/imalog.$$\
			 "$TCID: bios aggregate matches IMA boot aggregate."
		else
			tst_res TFAIL $LTPTMP/imalog.$$\
			 "$TCID: bios aggregate does not match IMA boot " \
				"aggregate."
		fi
	fi
	return $RC
}

# Probably cleaner to programmatically read the PCR values directly
# from the TPM, but that would require a TPM library. For now, use
# the PCR values from /sys/devices.
validate_pcr()
{
	ima_measurements=$SECURITYFS/ima/binary_runtime_measurements
	aggregate_pcr=`ima_measure $ima_measurements --validate`
	dev_pcrs=$1
	while read line ; do
		if [ "${line:0:6}" == "PCR-10" ]; then
			[ "${line:8:59}" == "${aggregate_pcr:25:59}" ]
				RC=$?
		fi
	done < $dev_pcrs
	return $RC
}

# Function:     test02
# Description	- Verify ima calculated aggregate PCR values matches
#		  actual PCR value.
test02()
{
	TCID="test02"
	TST_COUNT=2
	RC=0

#	Would be nice to know where the PCRs are located.  Is this safe?
	PCRS_PATH=`find /$SYSFS/devices/ | grep pcrs` || RC=$?
	if [ $RC -eq 0 ]; then
		validate_pcr $PCRS_PATH || RC=$?
		if [ $RC -eq 0 ]; then
			tst_res TPASS $LTPTMP/imalog.$$\
			 "$TCID: aggregate PCR value matches real PCR value."
		else
			tst_res TFAIL $LTPTMP/imalog.$$\
			 "$TCID: aggregate PCR value does not match" \
			 " real PCR value."
		fi
	else
		tst_res TFAIL $LTPTMP/imalog.$$\
		 "$TCID: TPM not enabled, no PCR value to validate"
	fi
	return $RC
}

# Function:     test03
# Description 	- Verify template hash value for IMA entry is correct.
test03()
{
	TCID="test03"
	TST_COUNT=3
	RC=0

	ima_measurements=$SECURITYFS/ima/binary_runtime_measurements
	aggregate_pcr=`ima_measure $ima_measurements --verify --validate` > /dev/null
	RC=$?
	if [ $RC -eq 0 ]; then
		tst_res TPASS $LTPTMP/imalog.$$\
		 "$TCID: verified IMA template hash values."
	else
		tst_res TFAIL $LTPTMP/imalog.$$\
		 "$TCID: error verifing IMA template hash values."
	fi
	return $RC
}

# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, and test functions.
EXIT_VAL=0

# set the testcases/bin directory
source `dirname $0`\/ima_setup.sh
setup || exit $RC

init || exit $RC
test01 || EXIT_VAL=$RC
test02 || EXIT_VAL=$RC
test03 || EXIT_VAL=$RC
exit $EXIT_VAL
