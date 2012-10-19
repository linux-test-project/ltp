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

	# Set the cert, key and combined cert and key filenames
	export TPM_CERTFILE="$LTPTMP/tst_tpm.cert"
	rm -f $TPM_CERTFILE 2>&1
	export TPM_KEYFILE="$LTPTMP/tst_tpm.key"
	rm -f $TPM_KEYFILE 2>&1
	export TPM_COMBFILE="$LTPTMP/tst_tpm.comb"
	rm -f $TPM_COMBFILE 2>&1

	# Set the OpenSSL configuration file
	export TPM_SSLCONF="$LTPBIN/tpmtoken_import_openssl.cnf"

	# Set known password values
	if [ -z "$P11_SO_PWD" ]
	then
		export P11_SO_PWD="P11 SO PWD"
	fi
	if [ -z "$P11_USER_PWD" ]
	then
		export P11_USER_PWD="P11 USER PWD"
	fi
	# This password needs to correspond to the passwords
	#   in the supplied OpenSSL configuration file
	if [ -z "$SSL_PWD" ]
	then
		export SSL_PWD="SSL PWD"
	fi

	tst_resm TINFO "INIT: Inititalizing tests."

	which tpmtoken_import 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brk TBROK $TPM_TMPFILE NULL \
			"Test: tpmtoken_import command does not exist. Reason:"
		return $RC
	fi

	which openssl 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brk TBROK $TPM_TMPFILE NULL \
			"Setup: openssl command does not exist. Reason:"
		return $RC
	fi
	openssl req -x509 -new -out $TPM_CERTFILE	\
		-newkey rsa:2048 -keyout $TPM_KEYFILE	\
		-sha1 -batch -config $TPM_SSLCONF
	if [ $RC -ne 0 ]
	then
		tst_brk TBROK $TPM_TMPFILE NULL \
			"Setup: unable to create certificate and/or key file. Reason:"
		return $RC
	fi
	cat $TPM_CERTFILE $TPM_KEYFILE 1>$TPM_COMBFILE 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brk TBROK $TPM_COMBFILE NULL \
			"Setup: unable to create combined certficate and key file. Reason:"
		return $RC
	fi

	return $RC
}

test01()
{
	RC=0				# Return value from commands
	export TCID=tpmtoken_import01	# Test ID
	export TST_COUNT=1		# Test number

	tpmtoken_import_tests_exp01.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_import -n $TCID $TPM_COMBFILE' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_import -n $TCID $TPM_COMBFILE' failed."
		RC=1
	fi
	return $RC
}

test02()
{
	RC=0				# Return value from commands
	export TCID=tpmtoken_import02	# Test ID
	export TST_COUNT=2		# Test number

	tpmtoken_import_tests_exp02.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_import -n $TCID $TPM_COMBFILE' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_import -n $TCID $TPM_COMBFILE' failed."
		RC=1
	fi
	return $RC
}

test03()
{
	RC=0				# Return value from commands
	export TCID=tpmtoken_import03	# Test ID
	export TST_COUNT=3		# Test number

	tpmtoken_import_tests_exp03.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_import -y -n $TCID $TPM_COMBFILE' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_import -y -n $TCID $TPM_COMBFILE' failed."
		RC=1
	fi
	return $RC
}

test04()
{
	RC=0				# Return value from commands
	export TCID=tpmtoken_import04	# Test ID
	export TST_COUNT=4		# Test number

	tpmtoken_import_tests_exp04.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_import -p -n $TCID $TPM_COMBFILE' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_import -p -n $TCID $TPM_COMBFILE' failed."
		RC=1
	fi
	return $RC
}

test05()
{
	RC=0				# Return value from commands
	export TCID=tpmtoken_import05	# Test ID
	export TST_COUNT=5		# Test number

	tpmtoken_import_tests_exp05.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_import -p -n $TCID $TPM_COMBFILE' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_import -p -n $TCID $TPM_COMBFILE' failed."
		RC=1
	fi
	return $RC
}

test06()
{
	RC=0				# Return value from commands
	export TCID=tpmtoken_import06	# Test ID
	export TST_COUNT=6		# Test number

	tpmtoken_import_tests_exp06.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_import -y -p -n $TCID $TPM_COMBFILE' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_import -y -p -n $TCID $TPM_COMBFILE' failed."
		RC=1
	fi
	return $RC
}

test07()
{
	RC=0				# Return value from commands
	export TCID=tpmtoken_import07	# Test ID
	export TST_COUNT=7		# Test number

	tpmtoken_import_tests_exp07.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_import -n $TCID $TPM_CERTFILE' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_import -n $TCID $TPM_CERTFILE' failed."
		RC=1
	fi
	return $RC
}

test08()
{
	RC=0				# Return value from commands
	export TCID=tpmtoken_import08	# Test ID
	export TST_COUNT=8		# Test number

	tpmtoken_import_tests_exp08.sh 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "'tpmtoken_import -n $TCID $TPM_KEYFILE' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_import -n $TCID $TPM_KEYFILE' failed."
		RC=1
	fi
	return $RC
}

test09()
{
	RC=0				# Return value from commands
	export TCID=tpmtoken_import09	# Test ID
	export TST_COUNT=9		# Test number

	#  The command should fail in this test case!
	tpmtoken_import -t key -n $TCID $TPM_CERTFILE 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "'tpmtoken_import -t key -n $TCID $TPM_CERTFILE' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_import -t key -n $TCID $TPM_CERTFILE' failed."
		RC=1
	fi
	return $RC
}

test10()
{
	RC=0				# Return value from commands
	export TCID=tpmtoken_import10	# Test ID
	export TST_COUNT=10		# Test number

	#  The command should fail in this test case!
	tpmtoken_import -t cert -n $TCID $TPM_KEYFILE 1>$TPM_TMPFILE 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_resm TPASS "'tpmtoken_import -t cert -n $TCID $TPM_KEYFILE' passed."
		RC=0
	else
		tst_res TFAIL $TPM_TMPFILE "'tpmtoken_import -t cert -n $TCID $TPM_KEYFILE' failed."
		RC=1
	fi
	return $RC
}

cleanup()
{
	rm -f $TPM_TMPFILE 2>&1
	rm -f $TPM_CERTFILE 2>&1
	rm -f $TPM_KEYFILE 2>&1
	rm -f $TPM_COMBFILE 2>&1
}

# Function:	main
#
# Description:	- Execute all tests, report results.
#
# Exit:		- zero on success
# 		- non-zero on failure.

TFAILCNT=0			# Set TFAILCNT to 0, increment on failure.
RC=0				# Return code from tests.

export TCID=tpmtoken_import	# Test ID
export TST_TOTAL=10		# Total numner of tests in this file.
export TST_COUNT=0		# Initialize identifier

if [ -n "$TPM_NOPKCS11" ]
then
	tst_resm TINFO "'tpmtoken_import' skipped."
	exit $TFAILCNT
fi

setup || exit $RC		# Exit if initializing testcases fails.

test01 || TFAILCNT=$(($TFAILCNT+1))
test02 || TFAILCNT=$(($TFAILCNT+1))
test03 || TFAILCNT=$(($TFAILCNT+1))
test04 || TFAILCNT=$(($TFAILCNT+1))
test05 || TFAILCNT=$(($TFAILCNT+1))
test06 || TFAILCNT=$(($TFAILCNT+1))
test07 || TFAILCNT=$(($TFAILCNT+1))
test08 || TFAILCNT=$(($TFAILCNT+1))
test09 || TFAILCNT=$(($TFAILCNT+1))
test10 || TFAILCNT=$(($TFAILCNT+1))

cleanup

exit $TFAILCNT
