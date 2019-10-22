#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
# Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
#
# Description:
# Register a new binary type and then check if binfmt_misc
# recognises the binary type in some conditions.
# 1) binfmt_misc should recognise the binary type when extension
#    or magic is matched.
# 2) binfmt_misc should not recognise the binary type when extension
#    or magic is mismatched.
# 3) binfmt_misc should not recognise the binary type when it is
#    disabled.
#
# Note:
# We use various delimiteris to register a new binary type.

TST_CNT=6
TST_TESTFUNC=do_test
TST_NEEDS_CMDS="which cat head"

. binfmt_misc_lib.sh

recognised_unrecognised()
{
	local file=$1
	local string="$2"

	eval $file >temp 2>&1
	if [ $? -ne 0 ] || ! grep -q "$string" temp; then
		tst_res TFAIL "Fail to recognise a binary type"
		return
	fi

	(echo 0 >"$mntpoint/$name") 2>/dev/null
	if [ $? -ne 0 ] || grep -q enable "$mntpoint/$name"; then
		tst_res TFAIL "Fail to disable a binary type"
		return
	fi

	eval $file >temp 2>&1
	if [ $? -eq 0 ] || grep -q "$string" temp; then
		tst_res TFAIL "Recognise a disabled binary type successfully"
		return
	fi

	tst_res TPASS "Recognise and unrecognise a binary type as expected"
}

unrecognised()
{
	local file=$1
	local string="$2"

	eval $file >temp 2>&1
	if [ $? -eq 0 ] || grep -q "$string" temp; then
		tst_res TFAIL "Recognise a binary type successfully"
	else
		tst_res TPASS "Fail to recognise a binary type"
	fi
}

verify_binfmt_misc()
{
	local delimiter=$(echo "$1" | head -c1)
	local name=$(echo "$1" | awk -F $delimiter '{print $2}')
	local ttype=$(echo "$1" | awk -F $delimiter '{print $3}')
	local tfile=$2
	local valid=$3
	local mntpoint=$(get_binfmt_misc_mntpoint)

	(echo "$1" >"$mntpoint/register") 2>/dev/null
	if [ $? -ne 0 -o ! -f "$mntpoint/$name" ]; then
		tst_res TFAIL "Fail to register a binary type"
		return
	fi

	[ "$ttype" = "E" ] && local tstring="This is test for extension"
	[ "$ttype" = "M" ] && local tstring="This is test for magic"

	[ "$valid" = "1" ] && recognised_unrecognised "$tfile" "$tstring"
	[ "$valid" = "0" ] && unrecognised "$tfile" "$tstring"

	remove_binary_type "$mntpoint/$name"
}

do_test()
{
	case $1 in
	1) verify_binfmt_misc ":textension:E::extension::$(which cat):" \
			      "$TST_DATAROOT/file.extension" "1";;
	2) verify_binfmt_misc ":tmagic:M:1:This::$(which cat):" \
			      "$TST_DATAROOT/file.magic" "1";;
	3) verify_binfmt_misc ".textension.E..extension..$(which cat)." \
			      "$TST_DATAROOT/file.extension" "1";;
	4) verify_binfmt_misc ",tmagic,M,1,This,,$(which cat)," \
			      "$TST_DATAROOT/file.magic" "1";;
	5) verify_binfmt_misc ":textension:E::ltp::$(which cat):" \
			      "$TST_DATAROOT/file.extension" "0";;
	6) verify_binfmt_misc ":tmagic:M:0:This::$(which cat):" \
			      "$TST_DATAROOT/file.magic" "0";;
	esac
}

tst_run
