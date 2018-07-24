#!/bin/sh
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
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Author: Petr Vorel <pvorel@suse.cz>

if_usage()
{
	echo "-c      Test command (ip, $IF_CMD)"
}

if_parse_args()
{
	case $1 in
	c) CMD="$2";;
	esac
}

if_setup()
{
	[ -n "$CMD" ] || tst_brk TBROK "IF_CMD variable not defined"
	if [ "$CMD" != 'ip' -a "$CMD" != "$IF_CMD" ]; then
		tst_brk TBROK "Missing or wrong -c parameter: '$CMD', use 'ip' or '$IF_CMD'"
	fi

	tst_test_cmds "$CMD"
	netstress_setup
	TST_CLEANUP="${TST_CLEANUP:-netstress_cleanup}"
}

if_cleanup_restore()
{
	netstress_cleanup
	restore_ipaddr
	restore_ipaddr rhost
}

TST_SETUP="${TST_SETUP:-if_setup}"
TST_TESTFUNC="test_body"
TST_PARSE_ARGS="if_parse_args"
TST_USAGE="if_usage"
TST_OPTS="c:"

. tst_net_stress.sh
