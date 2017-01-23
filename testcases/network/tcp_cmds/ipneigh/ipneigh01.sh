#!/bin/sh
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2000
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
# Test basic functionality of 'arp' and 'ip neigh'.

TCID=ipneigh01
NUMLOOPS=${NUMLOOPS:-50}
TST_TOTAL=2
. test_net.sh

do_setup()
{
	tst_require_root
	tst_check_cmds ip arp grep ping$TST_IPV6
}

do_test()
{
	local arp_show_cmd="$1"
	local arp_del_cmd="$2"

	local entry_name
	[ "$TST_IPV6" ] && entry_name="NDISC" || entry_name="ARP"

	tst_resm TINFO "Stress auto-creation of $entry_name cache entry"
	tst_resm TINFO "by pinging '$rhost' and deleting entry again"
	tst_resm TINFO "with '$arp_del_cmd'"

	for i in $(seq 1 $NUMLOOPS); do

		ping$TST_IPV6 -q -c1 $rhost > /dev/null

		local k
		local ret=1
		# wait for arp entry at least 3 seconds
		for k in $(seq 1 30); do
			$arp_show_cmd | grep -q $rhost
			if [ $? -eq 0 ]; then
				ret=0
				break;
			fi
			tst_sleep 100ms
		done

		[ "$ret" -ne 0 ] && \
			tst_brkm TFAIL "$entry_name entry '$rhost' not listed"

		$arp_del_cmd

		$arp_show_cmd | grep -q "${rhost}.*$(tst_hwaddr rhost)" && \
			tst_brkm TFAIL "'$arp_del_cmd' failed, entry has " \
				       "$(tst_hwaddr rhost)' $i/$NUMLOOPS"
	done

	tst_resm TPASS "verified adding/removing of $entry_name cache entry"
}

do_setup

rhost=$(tst_ipaddr rhost)

if [ -z "$TST_IPV6" ]; then
	do_test "arp -a" "arp -d $rhost"
else
	tst_resm TCONF "'arp cmd doesn't support IPv6, skipping test-case"
fi

do_test "ip neigh show" "ip neigh del $rhost dev $(tst_iface)"

tst_exit
