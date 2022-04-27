#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
#
# Test which command with some basic options.

TST_CNT=10
TST_SETUP=setup
TST_TESTFUNC=do_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="which"

setup()
{
	touch pname
	chmod +x pname
	PATH=$PATH:.

	mkdir bin
	touch bin/pname
	chmod +x bin/pname
	PATH=$PATH:./bin

	alias pname='pname -i'
}

which_verify()
{
	local IFS i j
	IFS="$IFS_FIRST_LEVEL"
	for i in $1; do
		found="no"
		IFS="$IFS_SECOND_LEVEL"
		for j in $i; do
			if grep -F -q "$j" temp; then
				found="yes"
			fi
		done
		if [ "$found" != "yes" ]; then
			echo "'$i' not found in:"
			cat temp
			echo
			return 1
		fi
	done
}

which_test()
{
	local which_op=$1
	local prog_name=$2

	local which_cmd="which $which_op $prog_name"

	if [ "$which_op" = "--read-alias" ] || [ "$which_op" = "-i" ] || \
		[ "$which_op" = "--skip-alias" ]; then
		which_cmd="alias | $which_cmd"
	fi

	eval ${which_cmd} >temp 2>&1
	if [ $? -ne 0 ]; then
		grep -q -E "unknown option|invalid option|Usage" temp
		if [ $? -eq 0 ]; then
			tst_res TCONF "'${which_cmd}' not supported."
			return
		fi

		tst_res TFAIL "'${which_cmd}' failed."
		cat temp
		return
	fi

	if [ $# -gt 2 ]; then
		shift 2
		which_verify "$@"
		if [ $? -ne 0 ]; then
			tst_res TFAIL "'${which_cmd}' failed, not expected."
			return
		fi
	fi

	tst_res TPASS "'${which_cmd}' passed."
}

IFS_FIRST_LEVEL='^'
IFS_SECOND_LEVEL='|'
do_test()
{
	case $1 in
	1) which_test "" "pname" "$PWD/pname|./pname";;
	2) which_test "-all" "pname" "$PWD/bin/pname|./bin/pname^$PWD/pname|./pname";;
	3) which_test "-a" "pname" "$PWD/bin/pname|./bin/pname^$PWD/pname|./pname";;
	4) which_test "--read-alias" "pname" "pname='pname -i'^$PWD/pname";;
	5) which_test "-i" "pname" "pname='pname -i'^$PWD/pname";;
	6) alias which='which --read-alias';
	   which_test "--skip-alias" "pname" "$PWD/pname";
	   unalias which;;
	7) which_test "--version";;
	8) which_test "-v";;
	9) which_test "-V";;
	10) which_test "--help";;
	esac
}

. tst_test.sh
tst_run
