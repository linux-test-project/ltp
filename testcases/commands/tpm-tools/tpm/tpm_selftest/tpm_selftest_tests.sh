#! /bin/sh
#
# Copyright (c) International Business Machines  Corp., 2005
#
# This program is free software;  you can redistribute it and#or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program;  if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

setup()
{
	RC=0				# Return code from commands.

	if [ -z "$LTPTMP" ] && [ -z "$TMPBASE" ]
	then
		LTPTMP="/tmp"
	else
		LTPTMP="$TMPBASE"
	fi

	export TPM_TMPFILE="$LTPTMP/tst_tpm.err"
	rm -f $TPM_TMPFILE 2>&1

	tst_resm TINFO "INIT: Inititalizing tests."

	which tpm_selftest 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brk TBROK $TPM_TMPFILE NULL \
			"Setup: tpm_selftest command does not exist. Reason:"
		return $RC
	fi

	return $RC
}

test01()
{
	RC=0				# Return value from commands
	export TCID=tpm_selftest01	# Test ID
	export TST_COUNT=1		# Test number

	tpm_selftest 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpm_selftest' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpm_selftest' failed."
		RC=1
	fi
	return $RC
}

test02()
{
	RC=0				# Return value from commands
	export TCID=tpm_selftest02	# Test ID
	export TST_COUNT=2		# Test number

	tpm_selftest -r 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpm_selftest -r' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpm_selftest -r' failed."
		RC=1
	fi
	return $RC
}

cleanup()
{
	rm -f $TPM_TMPFILE 2>&1
}

# Function:	main
#
# Description:	- Execute all tests, report results.
#
# Exit:		- zero on success
# 		- non-zero on failure.

TFAILCNT=0			# Set TFAILCNT to 0, increment on failure.
RC=0				# Return code from tests.

export TCID=tpm_selftest	# Test ID
export TST_TOTAL=2		# Total numner of tests in this file.
export TST_COUNT=0		# Initialize identifier

setup || exit $RC		# Exit if initializing testcases fails.

test01 || TFAILCNT=$(($TFAILCNT+1))
test02 || TFAILCNT=$(($TFAILCNT+1))

cleanup

exit $TFAILCNT
