#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2020 Microsoft Corporation
# Copyright (c) 2020-2025 Petr Vorel <pvorel@suse.cz>
# Author: Lachlan Sneff <t-josne@linux.microsoft.com>
#
# Verify that keys are measured correctly based on policy.

TST_NEEDS_CMDS="cmp cut grep sed"
TST_CNT=2
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_MIN_KVER="5.6"

REQUIRED_POLICY_CONTENT='keycheck.policy'

setup()
{
	local line

	require_policy_readable

	while read line; do
		if echo $line | grep -q 'template=' && ! echo $line | grep -q 'template=ima-buf'; then
			tst_brk TCONF "only template=ima-buf can be specified for KEY_CHECK"
		fi
	done < $IMA_POLICY
}

cleanup()
{
	tst_is_num $KEYRING_ID && keyctl clear $KEYRING_ID
}

# Based on https://lkml.org/lkml/2019/12/13/564.
# (450d0fd51564 - "IMA: Call workqueue functions to measure queued keys")
test1()
{
	local keycheck_lines i keyrings templates
	local test_file="file.txt" tmp_file="file2.txt"

	tst_res TINFO "verify key measurement for keyrings and templates specified in IMA policy"

	keycheck_lines=$(cat $tmp_file)
	keyrings=$(for i in $keycheck_lines; do echo "$i" | grep "keyrings" | \
		sed "s/\./\\\./g" | cut -d'=' -f2; done | sed ':a;N;$!ba;s/\n/|/g')
	if [ -z "$keyrings" ]; then
		tst_res TCONF "IMA policy has a keyring key-value specifier, but no specified keyrings"
		return
	fi

	templates=$(for i in $keycheck_lines; do echo "$i" | grep "template" | \
		cut -d'=' -f2; done | sed ':a;N;$!ba;s/\n/|/g')

	tst_res TINFO "keyrings: '$keyrings'"
	tst_res TINFO "templates: '$templates'"

	grep -E "($templates).*($keyrings)" $ASCII_MEASUREMENTS | while read line
	do
		local digest expected_digest algorithm

		digest=$(echo "$line" | cut -d' ' -f4 | cut -d':' -f2)
		algorithm=$(echo "$line" | cut -d' ' -f4 | cut -d':' -f1)
		keyring=$(echo "$line" | cut -d' ' -f5)

		echo "$line" | cut -d' ' -f6 | tst_hexdump -d > $test_file

		if ! expected_digest="$(compute_digest $algorithm $test_file)"; then
			tst_res TCONF "cannot compute digest for $algorithm"
			return
		fi

		if [ "$digest" != "$expected_digest" ]; then
			tst_res $IMA_FAIL "incorrect digest was found for $keyring keyring"
			return
		fi
	done

	tst_res TPASS "specified keyrings were measured correctly"
}

# Create a new keyring, import a certificate into it, and verify
# that the certificate is measured correctly by IMA.
test2()
{
	tst_require_cmds keyctl openssl

	require_evmctl "1.3.2"

	local cert_file="$TST_DATAROOT/x509_ima.der"
	local keyring_name="key_import_test"
	local temp_file="file.txt"

	tst_res TINFO "verify measurement of certificate imported into a keyring"

	KEYRING_ID=$(keyctl newring $keyring_name @s) || \
		tst_brk TBROK "unable to create a new keyring"

	if ! tst_is_num $KEYRING_ID; then
		tst_brk TBROK "unable to parse the new keyring id ('$KEYRING_ID')"
	fi

	evmctl import $cert_file $KEYRING_ID > /dev/null || \
		tst_brk TBROK "unable to import a certificate into $keyring_name keyring"

	grep $keyring_name $ASCII_MEASUREMENTS | tail -n1 | cut -d' ' -f6 | \
		tst_hexdump -d > $temp_file

	if [ ! -s $temp_file ]; then
		tst_res $IMA_FAIL "keyring $keyring_name not found in $ASCII_MEASUREMENTS"
		return
	fi

	if ! openssl x509 -in $temp_file -inform der > /dev/null; then
		tst_res $IMA_FAIL "logged certificate is not a valid x509 certificate"
		return
	fi

	if cmp -s $temp_file $cert_file; then
		tst_res TPASS "logged certificate matches the original"
	else
		tst_res $IMA_FAIL "logged certificate does not match original"
	fi
}

. ima_setup.sh
tst_run
