#!/bin/sh
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
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
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TCID=dccp01
TST_TOTAL=3
TST_CLEANUP="cleanup"
TST_NEEDS_TMPDIR=1

TST_USE_LEGACY_API=1
. tst_net.sh

cleanup()
{
	tst_rmdir
}

setup()
{
	tst_require_root
}

compare()
{
	local per=$(( $res0 * 100 / $res1 - 100 ))

	if [ "$per" -gt "100" -o "$per" -lt "-100" ]; then
		tst_resm TFAIL "$1 performance $per %"
	else
		tst_resm TPASS "$1 performance $per % in range -100 ... 100 %"
	fi
}

test_run()
{
	tst_resm TINFO "compare UDP/DCCP/UDP-Lite performance"

	tst_netload -H $(tst_ipaddr rhost) -T udp
	res0="$(cat tst_netload.res)"

	tst_netload -H $(tst_ipaddr rhost) -T dccp
	res1="$(cat tst_netload.res)"
	compare DCCP

	tst_netload -H $(tst_ipaddr rhost) -T udp_lite
	res1="$(cat tst_netload.res)"
	compare UDP-Lite
}

setup
test_run

tst_exit
