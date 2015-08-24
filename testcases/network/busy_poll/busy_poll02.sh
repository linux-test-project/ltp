#!/bin/sh

# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
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
TCID="busy_poll02"

. test_net.sh

cleanup()
{
	tst_rhost_run -c "pkill -9 tcp_fastopen\$"
	tst_rmdir

	sysctl -q -w net.core.busy_poll=$busy_poll_old
	tst_rhost_run -c "sysctl -q -w net.core.busy_poll=$rbusy_poll_old"
}

tst_require_root

tst_kvercmp 3 11 0
[ $? -eq 0 ] && tst_brkm TCONF "test must be run with kernel 3.11 or newer"

if [ ! -f "/proc/sys/net/core/busy_read" -a \
     ! -f "/proc/sys/net/core/busy_poll" ]; then
	tst_brkm TCONF "busy poll not configured, CONFIG_NET_RX_BUSY_POLL"
fi

set_busy_poll()
{
	local value=${1:-"0"}
	ROD_SILENT sysctl -q -w net.core.busy_poll=$value
	tst_rhost_run -s -c "sysctl -q -w net.core.busy_poll=$value"
}

tst_check_cmds pkill sysctl

tst_tmpdir

busy_poll_old="$(cat /proc/sys/net/core/busy_poll)"
rbusy_poll_old=$(tst_rhost_run -c 'cat /proc/sys/net/core/busy_poll')

TST_CLEANUP="cleanup"
trap "tst_brkm TBROK 'test interrupted'" INT

for x in 50 0; do
	tst_resm TINFO "set low latency busy poll to $x per socket"
	set_busy_poll $x
	tst_netload $(tst_ipaddr rhost) res_$x TFO -b $x || \
		tst_brkm TBROK "netload() failed"
	tst_resm TINFO "time spent is '$(cat res_$x)' ms"
done

poll_cmp=$(( 100 - ($(cat res_50) * 100) / $(cat res_0) ))

if [ "$poll_cmp" -lt 1 ]; then
	tst_resm TFAIL "busy poll result is '$poll_cmp' %"
else
	tst_resm TPASS "busy poll increased performance by '$poll_cmp' %"
fi

tst_exit
