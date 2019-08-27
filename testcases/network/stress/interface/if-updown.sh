#!/bin/sh
# Copyright (c) 2017-2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
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
# Author: Mitsuru Chinen <mitch@jp.ibm.com>

IF_CMD='ifconfig'
TST_CLEANUP="if_cleanup_restore"
. if-lib.sh

CHECK_INTERVAL=${CHECK_INTERVAL:-$(($IF_UPDOWN_TIMES / 20))}

test_body()
{
	local cmd="$CMD"
	local iface=$(tst_iface)

	tst_res TINFO "'$cmd' ups/downs $iface $IF_UPDOWN_TIMES times"
	tst_res TINFO "check connectivity interval is $CHECK_INTERVAL"

	local cnt=1
	while [ $cnt -le $IF_UPDOWN_TIMES ]; do
		case $cmd in
		ifconfig) ifconfig $iface down ;;
		ip) ip link set $iface down ;;
		esac
		if [ $? -ne 0 ]; then
			tst_res TFAIL "Failed to down $iface"
			return
		fi

		case $cmd in
		ifconfig) ifconfig $iface up ;;
		ip) ip link set $iface up ;;
		esac
		if [ $? -ne 0 ]; then
			tst_res TFAIL "Failed to up $iface"
			return
		fi

		check_connectivity_interval $cnt restore_ip || return

		cnt=$(($cnt + 1))
	done

	tst_res TPASS "Test is finished correctly"
}

tst_run
