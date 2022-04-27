#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
# Author: Mimi Zohar <zohar@linux.ibm.com>
#
# Test replacing the default integrity measurement policy.

TST_SETUP="setup"
TST_CNT=2

setup()
{
	require_policy_writable

	VALID_POLICY="$TST_DATAROOT/measure.policy"
	[ -f $VALID_POLICY ] || tst_brk TCONF "missing $VALID_POLICY"

	INVALID_POLICY="$TST_DATAROOT/measure.policy-invalid"
	[ -f $INVALID_POLICY ] || tst_brk TCONF "missing $INVALID_POLICY"
}

load_policy()
{
	local ret

	exec 2>/dev/null 4>$IMA_POLICY
	[ $? -eq 0 ] || exit 1

	cat $1 >&4 2> /dev/null
	ret=$?
	exec 4>&-

	[ $ret -eq 0 ] && \
		tst_res TINFO "IMA policy updated, please reboot after testing to restore settings"

	return $ret
}

test1()
{
	tst_res TINFO "verify that invalid policy isn't loaded"

	local p1

	require_policy_writable
	load_policy $INVALID_POLICY & p1=$!
	wait "$p1"
	if [ $? -ne 0 ]; then
		tst_res TPASS "didn't load invalid policy"
	else
		tst_res TFAIL "loaded invalid policy"
	fi
}

test2()
{
	tst_res TINFO "verify that policy file is not opened concurrently and able to loaded multiple times"

	local p1 p2 rc1 rc2

	require_policy_writable
	load_policy $VALID_POLICY & p1=$!
	load_policy $VALID_POLICY & p2=$!
	wait "$p1"; rc1=$?
	wait "$p2"; rc2=$?
	if [ $rc1 -eq 0 ] && [ $rc2 -eq 0 ]; then
		tst_res TFAIL "policy opened concurrently"
	elif [ $rc1 -eq 0 ] || [ $rc2 -eq 0 ]; then
		tst_res TPASS "policy was loaded just by one process and able to loaded multiple times"
	else
		tst_res TFAIL "problem loading or extending policy (may require policy to be signed)"
	fi
}

. ima_setup.sh
tst_run
