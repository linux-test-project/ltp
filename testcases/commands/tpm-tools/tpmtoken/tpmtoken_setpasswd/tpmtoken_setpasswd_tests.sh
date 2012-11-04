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

	# Set known password values
	if [ -z "$P11_SO_PWD" ]
	then
		export P11_SO_PWD="P11 SO PWD"
	fi
	if [ -z "$NEW_P11_SO_PWD" ]
	then
		export NEW_P11_SO_PWD="NEW P11 SO PWD"
	fi
	if [ -z "$P11_USER_PWD" ]
	then
		export P11_USER_PWD="P11 USER PWD"
	fi
	if [ -z "$NEW_P11_USER_PWD" ]
	then
		export NEW_P11_USER_PWD="NEW P11 USER PWD"
	fi

	tst_resm TINFO "INIT: Inititalizing tests."

	which tpmtoken_setpasswd 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brk TBROK $TPM_TMPFILE NULL \
			"Setup: tpmtoken_setpasswd command does not exist. Reason:"
		return $RC
	fi

	return $RC
}

test01()
{
	RC=0					# Return value from commands
	export TCID=tpmtoken_setpasswd01	# Test ID
	export TST_COUNT=1			# Test number

	tpmtoken_setpasswd_tests_exp01.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_setpasswd' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_setpasswd' failed."
		RC=1
	fi
	return $RC
}

test02()
{
	RC=0					# Return value from commands
	export TCID=tpmtoken_setpasswd02	# Test ID
	export TST_COUNT=2			# Test number

	tpmtoken_setpasswd_tests_exp02.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_setpasswd --security-officer' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_setpasswd --security-officer' failed."
		RC=1
	fi
	return $RC
}

test03()
{
	RC=0					# Return value from commands
	export TCID=tpmtoken_setpasswd03	# Test ID
	export TST_COUNT=3			# Test number

	tpmtoken_setpasswd_tests_exp03.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_setpasswd' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_setpasswd' failed."
		RC=1
	fi
	return $RC
}

test04()
{
	RC=0					# Return value from commands
	export TCID=tpmtoken_setpasswd04	# Test ID
	export TST_COUNT=4			# Test number

	tpmtoken_setpasswd_tests_exp04.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_setpasswd --security-officer' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_setpasswd --security-officer' failed."
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

export TCID=tpmtoken_setpasswd	# Test ID
export TST_TOTAL=4		# Total numner of tests in this file.
export TST_COUNT=0		# Initialize identifier

if [ -n "$TPM_NOPKCS11" ]
then
	tst_resm TINFO "'tpmtoken_setpasswd' skipped."
	exit $TFAILCNT
fi

setup || exit $RC		# Exit if initializing testcases fails.

test01 || TFAILCNT=$(($TFAILCNT+1))
test02 || TFAILCNT=$(($TFAILCNT+1))
test03 || TFAILCNT=$(($TFAILCNT+1))
test04 || TFAILCNT=$(($TFAILCNT+1))

cleanup

exit $TFAILCNT
