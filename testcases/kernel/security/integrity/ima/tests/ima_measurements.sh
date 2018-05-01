#!/bin/sh
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Author: Mimi Zohar, zohar@ibm.vnet.ibm.com
#
# Verify that measurements are added to the measurement list based on policy.

TST_NEEDS_CMDS="awk"
TST_SETUP="setup"
TST_CNT=3
TST_NEEDS_DEVICE=1

. ima_setup.sh

setup()
{
	DEFAULT_DIGEST_OLD_FORMAT="sha1"
	TEST_FILE="$PWD/test.txt"

	POLICY="$IMA_DIR/policy"
	[ -f "$POLICY" ] || tst_res TINFO "not using default policy"

	DIGEST_INDEX=
	grep -q "ima-ng" $ASCII_MEASUREMENTS && DIGEST_INDEX=1
	grep -q "ima-sig" $ASCII_MEASUREMENTS && DIGEST_INDEX=2

	tst_res TINFO "IMA measurement tests assume tcb policy to be loaded (ima_policy=tcb)"
}

# TODO: find support for rmd128 rmd256 rmd320 wp256 wp384 tgr128 tgr160
compute_hash()
{
	local digest="$1"
	local file="$2"

	hash="$(${digest}sum $file 2>/dev/null | cut -f1 -d ' ')"
	[ -n "$hash" ] && { echo $hash; return; }

	hash="$(openssl $digest $file 2>/dev/null | cut -f2 -d ' ')"
	[ -n "$hash" ] && { echo $hash; return; }

	# uncommon ciphers
	local arg="$digest"
	case "$digest" in
	tgr192) arg="tiger" ;;
	wp512) arg="whirlpool" ;;
	esac

	hash="$(rhash --$arg $file 2>/dev/null | cut -f1 -d ' ')"
	[ -n "$hash" ] && { echo $hash; return; }
}

ima_check()
{
	local digest="$DEFAULT_DIGEST_OLD_FORMAT"
	local hash expected_hash line

	# need to read file to get updated $ASCII_MEASUREMENTS
	cat $TEST_FILE > /dev/null

	line="$(grep $TEST_FILE $ASCII_MEASUREMENTS | tail -1)"
	[ -n "$line" ] || tst_res TFAIL "cannot find measurement for '$TEST_FILE'"

	[ "$DIGEST_INDEX" ] && digest="$(echo "$line" | awk '{print $(NF-'$DIGEST_INDEX')}' | cut -d ':' -f 1)"
	hash="$(echo "$line" | awk '{print $(NF-1)}' | cut -d ':' -f 2)"

	tst_res TINFO "computing hash for $digest digest"
	expected_hash="$(compute_hash $digest $TEST_FILE)" || \
		{ tst_res TCONF "cannot compute hash for '$digest' digest"; return; }

	if [ "$hash" = "$expected_hash" ]; then
		tst_res TPASS "correct hash found"
	else
		tst_res TFAIL "hash not found"
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
