#!/bin/sh
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
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
#

TST_TOTAL=1
TCID="busy_poll03"
TST_NEEDS_TMPDIR=1

. test_net.sh
. busy_poll_lib.sh

cleanup()
{
	tst_rmdir

	sysctl -q -w net.core.busy_poll=$busy_poll_old
	tst_rhost_run -c "sysctl -q -w net.core.busy_poll=$rbusy_poll_old"
}

set_busy_poll()
{
	local value=${1:-"0"}
	ROD_SILENT sysctl -q -w net.core.busy_poll=$value
	tst_rhost_run -s -c "sysctl -q -w net.core.busy_poll=$value"
}

busy_poll_old="$(cat /proc/sys/net/core/busy_poll)"
rbusy_poll_old=$(tst_rhost_run -c 'cat /proc/sys/net/core/busy_poll')

TST_CLEANUP="cleanup"
trap "tst_brkm TBROK 'test interrupted'" INT

for x in 50 0; do
	tst_resm TINFO "set low latency busy poll to $x per socket"
	set_busy_poll $x
	tst_netload -H $(tst_ipaddr rhost) -d res_$x -b $x -T udp
done

poll_cmp=$(( 100 - ($(cat res_50) * 100) / $(cat res_0) ))

if [ "$poll_cmp" -lt 1 ]; then
	tst_resm TFAIL "busy poll result is '$poll_cmp' %"
else
	tst_resm TPASS "busy poll increased performance by '$poll_cmp' %"
fi

tst_exit
