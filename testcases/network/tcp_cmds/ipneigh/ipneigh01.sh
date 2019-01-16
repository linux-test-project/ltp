#!/bin/sh
# Copyright (c) 2018 SUSE Linux GmbH
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

NUMLOOPS=${NUMLOOPS:-50}
TST_TESTFUNC=do_test
TST_SETUP=do_setup
TST_OPTS="c:"
TST_PARSE_ARGS="parse_args"
TST_USAGE="usage"
TST_NEEDS_ROOT=1
. tst_net.sh

do_setup()
{
	case $CMD in
	ip)
		SHOW_CMD="ip neigh show"
		DEL_CMD="ip neigh del $(tst_ipaddr rhost) dev $(tst_iface)"
		;;
	arp)
		if [ -n "$TST_IPV6" ]; then
			tst_brk TCONF "'arp' doesn't support IPv6"
		fi
		SHOW_CMD="arp -a"
		DEL_CMD="arp -d $(tst_ipaddr rhost) -i $(tst_iface)"
		;;
	*)
		tst_brk TBROK "unknown or missing command, use -c [ arp | ip ]"
		;;
	esac

	tst_test_cmds $CMD ping$TST_IPV6
}

usage()
{
	echo "-c [ arp | ip ] Test command"
}

parse_args()
{
	case $1 in
	c) CMD="$2" ;;
	esac
}

do_test()
{
	local entry_name="ARP"
	[ "$TST_IPV6" ] && entry_name="NDISC"

	tst_res TINFO "stress auto-creation $entry_name cache entry deleted with '$CMD' $NUMLOOPS times"

	for i in $(seq 1 $NUMLOOPS); do

		ping$TST_IPV6 -q -c1 $(tst_ipaddr rhost) -I $(tst_iface) > /dev/null || \
			tst_brk TFAIL "cannot ping $(tst_ipaddr rhost)"

		local k
		local ret=1
		for k in $(seq 1 30); do
			$SHOW_CMD | grep -q $(tst_ipaddr rhost)
			if [ $? -eq 0 ]; then
				ret=0
				break;
			fi
			tst_sleep 100ms
		done

		[ "$ret" -ne 0 ] && \
			tst_brk TFAIL "$entry_name entry '$(tst_ipaddr rhost)' not listed"

		$DEL_CMD || tst_brk TFAIL "fail to delete entry"

		$SHOW_CMD | grep -q "$(tst_ipaddr rhost).*$(tst_hwaddr rhost)" && \
			tst_brk TFAIL "'$DEL_CMD' failed, entry has " \
				"$(tst_hwaddr rhost)' $i/$NUMLOOPS"
	done

	tst_res TPASS "verified adding/removing $entry_name cache entry"
}

tst_run
