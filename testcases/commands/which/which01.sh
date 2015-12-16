#!/bin/sh
#
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
# the GNU General Public License for more details.
#
# Test which command with some basic options.
#

TCID=which01
TST_TOTAL=10
. test.sh

setup()
{
	tst_check_cmds which

	tst_tmpdir

	TST_CLEANUP="cleanup"

	touch pname
	chmod +x pname
	PATH=$PATH:.

	mkdir bin
	touch bin/pname
	chmod +x bin/pname
	PATH=$PATH:./bin

	alias pname='pname -i'
}

cleanup()
{
	tst_rmdir
}

which_verify()
{
	until [ -z "$1" ]
	do
		grep -q "$1" temp
		if [ $? -ne 0 ]; then
			echo "'$1' not found in:"
			cat temp
			echo
			return 1
		fi
		shift
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
			tst_resm TCONF "'${which_cmd}' not supported."
			return
		fi

		tst_resm TFAIL "'${which_cmd}' failed."
		cat temp
		return
	fi

	if [ $# -gt 2 ]; then
		shift 2
		which_verify "$@"
		if [ $? -ne 0 ]; then
			tst_resm TFAIL "'${which_cmd}' failed, not expected."
			return
		fi
	fi

	tst_resm TPASS "'${which_cmd}' passed."
}

setup

which_test "" "pname" "$PWD/pname"
which_test "--all" "pname" "$PWD/bin/pname" "$PWD/pname"
which_test "-a" "pname" "$PWD/bin/pname" "$PWD/pname"
which_test "--read-alias" "pname" "pname='pname -i'" "$PWD/pname"
which_test "-i" "pname" "pname='pname -i'" "$PWD/pname"

alias which='which --read-alias'
which_test "--skip-alias" "pname" "$PWD/pname"

which_test "--version"
which_test "-v"
which_test "-V"
which_test "--help"

tst_exit
