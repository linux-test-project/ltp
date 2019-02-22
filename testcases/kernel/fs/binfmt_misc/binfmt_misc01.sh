#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (c) 2019 FUJITSU LIMITED. All rights reserved.
# Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
#
# Description:
# Use various invalid inputs to register a new binary type.
# 1) Invalid format of string fails to register a new binary type.
# 2) Invalid type fails to register a new binary type.
# 3) Invalid name containing slashes fails to register a new
#    binary type.
# 4) If extension matching is chosen, invalid magic containing
#    slashes fails to register a new binary type.
# 5) If magic matching is chosen, invalid offset(e.g. -1 and
#    2500000000) fails to register a new binary type.
# 6) Invalid flag fails to register a new binary type.
#
# Note:
# This is also a regression test for the following kernel bug:
# '5cc41e099504 ("fs/binfmt_misc.c: do not allow offset overflow")'


TST_CNT=9
TST_TESTFUNC=do_test
TST_NEEDS_CMDS="cat"

. binfmt_misc_lib.sh

verify_binfmt_misc()
{
	local name=$(echo "$1" | awk -F ':' '{print $2}')
	local mntpoint=$(get_binfmt_misc_mntpoint)

	(echo "$1" >"$mntpoint/register") 2>/dev/null
	if [ $? -ne 0 -a ! -f "$mntpoint/$name" ]; then
		tst_res TPASS "Failed to register a binary type"
		return
	fi

	# Trigger kernel crash reliably by cat command.
	cat "$mntpoint/$name" >/dev/null 2>&1
	tst_res TFAIL "Register a binary type successfully"

	[ -f "$mntpoint/$name" ] && \
		remove_binary_type "$mntpoint/$name"
}

do_test()
{
	case $1 in
	1) verify_binfmt_misc ".textension,E,,ltp,,$(which cat),";;
	2) verify_binfmt_misc ":tnone:X::ltp::$(which cat):";;
	3) verify_binfmt_misc ":textension/:E::ltp::$(which cat):";;
	4) verify_binfmt_misc ":tmagic/:M::ltp::$(which cat):";;
	5) verify_binfmt_misc ":textension:E::ltp/::$(which cat):";;
	6) verify_binfmt_misc ":tmagic:M:-1:ltp::$(which cat):";;
	7) verify_binfmt_misc ":tmagic:M:2500000000:ltp::$(which cat):";;
	8) verify_binfmt_misc ":textension:E::ltp::$(which cat):A";;
	9) verify_binfmt_misc ":tmagic:M::ltp::$(which cat):A";;
	esac
}

tst_run
