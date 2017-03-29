#!/bin/sh
#
# Copyright (c) 2017 Fujitsu Ltd.
# Ported: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program, if not, see <http://www.gnu.org/licenses/>.
#
# This is a regression test about potential uninitialized variable,
# the test can crash the buggy kernel, and the bug has been fixed in:
#
#   commit 38327424b40bcebe2de92d07312c89360ac9229a
#   Author: Dan Carpenter <dan.carpenter@oracle.com>
#   Date:   Thu Jun 16 15:48:57 2016 +0100
#
#   KEYS: potential uninitialized variable
#

TST_ID="keyctl01"
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_TESTFUNC=do_test
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="keyctl"
. tst_test.sh

setup()
{
	if tst_kvcmp -le 2.6.33; then
		tst_brk TCONF "Kernel newer than 2.6.33 is needed"
	fi

	PATH_KEYSTAT="/proc/key-users"
	PATH_KEYQUOTA="/proc/sys/kernel/keys/root_maxbytes"

	if [ ! -f "$PATH_KEYSTAT" ] || [ ! -f "$PATH_KEYQUOTA" ]; then
		tst_brk TCONF "'${PATH_KEYSTAT}' or '${PATH_KEYQUOTA}' \
			does not exist"
	fi

	ORIG_KEYSZ=`awk -F' +|/' '/ 0:/ {print $8}' $PATH_KEYSTAT`
	ORIG_MAXKEYSZ=`cat $PATH_KEYQUOTA`
}

cleanup()
{
	if [ -n "$ORIG_MAXKEYSZ" ]; then
		echo $ORIG_MAXKEYSZ >$PATH_KEYQUOTA
	fi
}

do_test()
{
	local maxkeysz=$((ORIG_KEYSZ + 100))

	while true
	do
		echo $maxkeysz >$PATH_KEYQUOTA

		keyctl request2 user debug:fred negate @t >temp 2>&1
		grep -q -E "quota exceeded" temp
		if [ $? -eq 0 ]; then
			break
		fi

		local key=`keyctl show | awk '/debug:fred/ {print $1}'`
		if [ -n "$key" ]; then
			keyctl unlink $key @s >/dev/null
			tst_sleep 50ms
		fi

		((maxkeysz -= 4))
	done

	tst_res TPASS "Bug not reproduced"
}

tst_run
