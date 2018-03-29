#!/bin/sh
# Copyright (c) 2018 Oracle and/or its affiliates.
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

TST_TOTAL=1
TCID="bind_noport"
TST_NEEDS_TMPDIR=1

TST_USE_LEGACY_API=1
. tst_net.sh

cleanup()
{
	tst_rmdir
}

if tst_kvcmp -lt "4.2"; then
	tst_brkm TCONF "test must be run with kernel 4.2+"
fi

trap "tst_brkm TBROK 'test interrupted'" INT
TST_CLEANUP="cleanup"

do_test()
{
	local types="tcp udp udp_lite dccp"
	TST_NETLOAD_CLN_NUMBER=10
	TST_NETLOAD_CLN_REQUESTS=1000

	tst_resm TINFO "test IP_BIND_ADDRESS_NO_PORT with $types sockets"

	for t in $types; do
		tst_netload -T $t -S $(tst_ipaddr) -H $(tst_ipaddr rhost)
	done
}

do_test

tst_exit
