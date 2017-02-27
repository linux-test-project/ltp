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

TST_CLEANUP="cleanup"

. test_net.sh

ipver=${TST_IPV6:-4}

IPV4_NET16_UNUSED=${IPV4_NET16_UNUSED:-"10.23"}
IPV6_NET32_UNUSED=${IPV6_NET32_UNUSED:-"fd00:23"}

setup()
{
	tst_require_root
	tst_check_cmds ip pgrep pkill
	trap "tst_brkm TBROK 'test interrupted'" INT
}

cleanup()
{
	# Stop the background TCP traffic
	pkill -13 -x netstress
	tst_rhost_run -c "pkill -13 -x netstress"
}

make_background_tcp_traffic()
{
	port=$(tst_get_unused_port ipv${ipver} stream)
	netstress -R 3 -g $port > /dev/null 2>&1 &
	tst_rhost_run -b -c "netstress -l -H $(tst_ipaddr) -g $port"
}

check_connectivity()
{
	local cnt="$1"
	local restore="$2"

	[ $CHECK_INTERVAL -eq 0 ] && return
	[ $(($cnt % $CHECK_INTERVAL)) -ne 0 ] && return

	tst_resm TINFO "check connectivity through $(tst_iface) on step $cnt"

	[ -n "$restore" ] && restore_ipaddr

	tst_ping
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "$(tst_iface) is broken"
		return 1
	fi
	return 0
}

restore_ipaddr()
{
	tst_restore_ipaddr || return $?
	tst_wait_ipv6_dad
}
