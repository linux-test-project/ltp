#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018-2021 Petr Vorel <pvorel@suse.cz>
# Author: Mimi Zohar <zohar@linux.ibm.com>
#
# Verify that measurements are added to the measurement list based on policy.

TST_NEEDS_CMDS="awk cut sed"
TST_SETUP="setup"
TST_CNT=3
TST_NEEDS_DEVICE=1

. ima_setup.sh

setup()
{
	require_ima_policy_cmdline "tcb"

	TEST_FILE="$PWD/test.txt"
	[ -f "$IMA_POLICY" ] || tst_res TINFO "not using default policy"
}

check_iversion_support()
{
	local device mount fs

	tst_kvcmp -ge "4.16" && return 0

	device="$(df . | sed -e 1d | cut -f1 -d ' ')"
	mount="$(grep $device /proc/mounts | head -1)"
	fs="$(echo $mount | awk '{print $3'})"

	case "$fs" in
	ext[2-4])
		if ! echo "$mount" | grep -q -w "i_version"; then
			tst_res TCONF "device '$device' is not mounted with iversion, please mount it with 'mount $device -o remount,iversion'"
			return 1
		fi
		;;
	xfs)
		if dmesg | grep -q "XFS.*Mounting V[1-4] Filesystem"; then
			tst_res TCONF "XFS Filesystem >= V5 required for iversion support"
			return 1
		fi
		;;
	'')
		tst_res TWARN "could not find mount info for device '$device'"
		;;
	esac

	return 0
}

test1()
{
	tst_res TINFO "verify adding record to the IMA measurement list"
	ROD echo "$(cat /proc/uptime) this is a test file" \> $TEST_FILE
	ima_check $TEST_FILE
}

test2()
{

	tst_res TINFO "verify updating record in the IMA measurement list"
	check_iversion_support || return
	ROD echo "$(cat /proc/uptime) modified file" \> $TEST_FILE
	ima_check $TEST_FILE
}

test3()
{
	local user="nobody"
	local dir="$PWD/user"
	local file="$dir/test.txt"

	# Default policy does not measure user files
	tst_res TINFO "verify not measuring user files"
	tst_check_cmds sudo || return

	if ! id $user >/dev/null 2>/dev/null; then
		tst_res TCONF "missing system user $user (wrong installation)"
		return
	fi

	[ -d "$dir" ] || mkdir -m 0700 $dir
	chown $user $dir
	cd $dir
	# need to read file to get updated $ASCII_MEASUREMENTS
	sudo -n -u $user sh -c "echo $(cat /proc/uptime) user file > $file; cat $file > /dev/null"
	cd ..

	EXPECT_FAIL "grep $file $ASCII_MEASUREMENTS"
}

tst_run
