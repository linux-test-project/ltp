#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Microsoft Corporation
# Copyright (c) 2020 Petr Vorel <pvorel@suse.cz>
# Author: Lachlan Sneff <t-josne@linux.microsoft.com>
#
# Verify that keys are measured correctly based on policy.

TST_NEEDS_CMDS="cmp cut grep sed tr xxd"
TST_CNT=2
TST_NEEDS_DEVICE=1
TST_CLEANUP=cleanup

. ima_setup.sh

cleanup()
{
	tst_is_num $KEYRING_ID && keyctl clear $KEYRING_ID
}

# Based on https://lkml.org/lkml/2019/12/13/564.
# (450d0fd51564 - "IMA: Call workqueue functions to measure queued keys")
test1()
{
	local keyrings keycheck_lines keycheck_line templates
	local func='func=KEY_CHECK'
	local buf='template=ima-buf'
	local pattern="($func.*$buf|$buf.*$func)"
	local test_file="file.txt"

	tst_res TINFO "verifying key measurement for keyrings and templates specified in IMA policy file"

	require_ima_policy_content "$pattern" '-Eq'
	keycheck_lines=$(check_ima_policy_content "$pattern" '-E')
	keycheck_line=$(echo "$keycheck_lines" | grep "keyrings" | head -n1)

	if [ -z "$keycheck_line" ]; then
		tst_res TCONF "IMA policy does not specify a keyrings to check"
		return
	fi

	keyrings=$(echo "$keycheck_line" | tr " " "\n" | grep "keyrings" | \
		sed "s/\./\\\./g" | cut -d'=' -f2)
	if [ -z "$keyrings" ]; then
		tst_res TCONF "IMA policy has a keyring key-value specifier, but no specified keyrings"
		return
	fi

	templates=$(echo "$keycheck_line" | tr " " "\n" | grep "template" | \
		cut -d'=' -f2)

	grep -E "($templates).*($keyrings)" $ASCII_MEASUREMENTS | while read line
	do
		local digest expected_digest algorithm

		digest=$(echo "$line" | cut -d' ' -f4 | cut -d':' -f2)
		algorithm=$(echo "$line" | cut -d' ' -f4 | cut -d':' -f1)
		keyring=$(echo "$line" | cut -d' ' -f5)

		echo "$line" | cut -d' ' -f6 | xxd -r -p > $test_file

		if ! expected_digest="$(compute_digest $algorithm $test_file)"; then
			tst_res TCONF "cannot compute digest for $algorithm"
			return
		fi

		if [ "$digest" != "$expected_digest" ]; then
			tst_res TFAIL "incorrect digest was found for $keyring keyring"
			return
		fi
	done

	tst_res TPASS "specified keyrings were measured correctly"
}

# Create a new keyring, import a certificate into it, and verify
# that the certificate is measured correctly by IMA.
test2()
{
	tst_require_cmds evmctl keyctl openssl

	local cert_file="$TST_DATAROOT/x509_ima.der"
	local keyring_name="key_import_test"
	local temp_file="file.txt"

	tst_res TINFO "verify measurement of certificate imported into a keyring"

	if ! check_ima_policy_content "^measure.*func=KEY_CHECK.*keyrings=.*$keyring_name"; then
		tst_brk TCONF "IMA policy does not contain $keyring_name keyring"
	fi

	KEYRING_ID=$(keyctl newring $keyring_name @s) || \
		tst_brk TBROK "unable to create a new keyring"

	if ! tst_is_num $KEYRING_ID; then
		tst_brk TBROK "unable to parse the new keyring id ('$KEYRING_ID')"
	fi

	evmctl import $cert_file $KEYRING_ID > /dev/null || \
		tst_brk TBROK "unable to import a certificate into $keyring_name keyring"

	grep $keyring_name $ASCII_MEASUREMENTS | tail -n1 | cut -d' ' -f6 | \
		xxd -r -p > $temp_file

	if [ ! -s $temp_file ]; then
		tst_res TFAIL "keyring $keyring_name not found in $ASCII_MEASUREMENTS"
		return
	fi

	if ! openssl x509 -in $temp_file -inform der > /dev/null; then
		tst_res TFAIL "logged certificate is not a valid x509 certificate"
		return
	fi

	if cmp -s $temp_file $cert_file; then
		tst_res TPASS "logged certificate matches the original"
	else
		tst_res TFAIL "logged certificate does not match original"
	fi
}

tst_run
