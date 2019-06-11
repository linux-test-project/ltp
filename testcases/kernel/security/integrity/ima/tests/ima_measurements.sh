#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018-2019 Petr Vorel <pvorel@suse.cz>
# Author: Mimi Zohar <zohar@linux.ibm.com>
#
# Verify that measurements are added to the measurement list based on policy.

TST_NEEDS_CMDS="awk cut"
TST_SETUP="setup"
TST_CNT=3
TST_NEEDS_DEVICE=1

. ima_setup.sh

setup()
{
	check_ima_policy "tcb"

	TEST_FILE="$PWD/test.txt"
	POLICY="$IMA_DIR/policy"
	[ -f "$POLICY" ] || tst_res TINFO "not using default policy"
	DIGEST_INDEX=

	local template="$(tail -1 $ASCII_MEASUREMENTS | cut -d' ' -f 3)"
	local i

	# parse digest index
	# https://www.kernel.org/doc/html/latest/security/IMA-templates.html#use
	case "$template" in
	ima|ima-ng|ima-sig) DIGEST_INDEX=4 ;;
	*)
		# using ima_template_fmt kernel parameter
		local IFS="|"
		i=4
		for word in $template; do
			if [ "$word" = 'd' -o "$word" = 'd-ng' ]; then
				DIGEST_INDEX=$i
				break
			fi
			i=$((i+1))
		done
	esac

	[ -z "$DIGEST_INDEX" ] && tst_brk TCONF \
		"Cannot find digest index (template: '$template')"
}

# TODO: find support for rmd128 rmd256 rmd320 wp256 wp384 tgr128 tgr160
compute_digest()
{
	local algorithm="$1"
	local file="$2"
	local digest

	digest="$(${algorithm}sum $file 2>/dev/null | cut -f1 -d ' ')"
	if [ -n "$digest" ]; then
		echo "$digest"
		return 0
	fi

	digest="$(openssl $algorithm $file 2>/dev/null | cut -f2 -d ' ')"
	if [ -n "$digest" ]; then
		echo "$digest"
		return 0
	fi

	# uncommon ciphers
	local arg="$algorithm"
	case "$algorithm" in
	tgr192) arg="tiger" ;;
	wp512) arg="whirlpool" ;;
	esac

	digest="$(rdigest --$arg $file 2>/dev/null | cut -f1 -d ' ')"
	if [ -n "$digest" ]; then
		echo "$digest"
		return 0
	fi
	return 1
}

ima_check()
{
	local delimiter=':'
	local algorithm digest expected_digest line

	# need to read file to get updated $ASCII_MEASUREMENTS
	cat $TEST_FILE > /dev/null

	line="$(grep $TEST_FILE $ASCII_MEASUREMENTS | tail -1)"
	if [ -z "$line" ]; then
		tst_res TFAIL "cannot find measurement record for '$TEST_FILE'"
		return
	fi
	tst_res TINFO "measurement record: '$line'"

	digest=$(echo "$line" | cut -d' ' -f $DIGEST_INDEX)
	if [ -z "$digest" ]; then
		tst_res TFAIL "cannot find digest (index: $DIGEST_INDEX)"
		return
	fi

	if [ "${digest#*$delimiter}" != "$digest" ]; then
		algorithm=$(echo "$digest" | cut -d $delimiter -f 1)
		digest=$(echo "$digest" | cut -d $delimiter -f 2)
	else
		case "${#digest}" in
		32) algorithm="md5" ;;
		40) algorithm="sha1" ;;
		*)
			tst_res TFAIL "algorithm must be either md5 or sha1 (digest: '$digest')"
			return ;;
		esac
	fi
	if [ -z "$algorithm" ]; then
		tst_res TFAIL "cannot find algorithm"
		return
	fi
	if [ -z "$digest" ]; then
		tst_res TFAIL "cannot find digest"
		return
	fi

	tst_res TINFO "computing digest for $algorithm algorithm"
	expected_digest="$(compute_digest $algorithm $TEST_FILE)" || \
		tst_brk TCONF "cannot compute digest for $algorithm algorithm"

	if [ "$digest" = "$expected_digest" ]; then
		tst_res TPASS "correct digest found"
	else
		tst_res TFAIL "digest not found"
	fi
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
	ROD echo "$(date) this is a test file" \> $TEST_FILE
	ima_check
}

test2()
{

	tst_res TINFO "verify updating record in the IMA measurement list"
	check_iversion_support || return
	ROD echo "$(date) modified file" \> $TEST_FILE
	ima_check
}

test3()
{
	local user="nobody"
	local dir="$PWD/user"
	local file="$dir/test.txt"

	# Default policy does not measure user files
	tst_res TINFO "verify not measuring user files"
	tst_check_cmds sudo

	if ! id $user >/dev/null 2>/dev/null; then
		tst_res TCONF "missing system user $user (wrong installation)"
		return
	fi

	mkdir -m 0700 $dir
	chown $user $dir
	cd $dir
	# need to read file to get updated $ASCII_MEASUREMENTS
	sudo -n -u $user sh -c "echo $(date) user file > $file; cat $file > /dev/null"
	cd ..

	EXPECT_FAIL "grep $file $ASCII_MEASUREMENTS"
}

tst_run
