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
# Test replacing the default integrity measurement policy.

TST_SETUP="setup"
TST_CNT=3

. ima_setup.sh

setup()
{
	IMA_POLICY="$IMA_DIR/policy"
	[ -f $IMA_POLICY ] || \
		tst_brk TCONF "IMA policy already loaded and kernel not configured to enable multiple writes it"

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

	cat $1 |
	while read line; do
		if [ "${line#\#}" = "${line}" ]; then
			echo "$line" >&4 2> /dev/null
			if [ $? -ne 0 ]; then
				exec 4>&-
				return 1
			fi
		fi
	done
	ret=$?

	[ $ret -eq 0 ] && \
		tst_res TINFO "IMA policy updated, please reboot after testing to restore settings"

	return $ret
}

test1()
{
	tst_res TINFO "verify that invalid policy isn't loaded"

	local p1

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
	tst_res TINFO "verify that policy file is not opened concurrently"

	local p1 p2 rc1 rc2

	load_policy $VALID_POLICY & p1=$!
	load_policy $VALID_POLICY & p2=$!
	wait "$p1"; rc1=$?
	wait "$p2"; rc2=$?
	if [ $rc1 -eq 0 ] && [ $rc2 -eq 0 ]; then
		tst_res TFAIL "policy opened concurrently"
	elif [ $rc1 -eq 0 ] || [ $rc2 -eq 0 ]; then
		tst_res TPASS "policy was loaded just by one process"
	else
		tst_res TFAIL "problem loading policy"
	fi
}

test3()
{
	tst_res TINFO "verify that invalid policy isn't loaded"

	local p1

	load_policy $INVALID_POLICY & p1=$!
	wait "$p1"
	if [ $? -ne 0 ]; then
		tst_res TPASS "didn't replace valid policy"
	else
		tst_res TFAIL "replaced valid policy"
	fi
}

tst_run
