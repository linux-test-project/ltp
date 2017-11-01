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
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TCID=dccp_ipsec
TST_TOTAL=3
TST_NEEDS_TMPDIR=1
TST_CLEANUP="tst_ipsec_cleanup"

. ipsec_lib.sh

do_setup()
{
	# Configure SAD/SPD
	if [ -n "$IPSEC_MODE" -a -n "$IPSEC_PROTO" ]; then
		tst_ipsec lhost $(tst_ipaddr) $(tst_ipaddr rhost)
		tst_ipsec rhost $(tst_ipaddr rhost) $(tst_ipaddr)
	fi
}

do_test()
{
	for p in $IPSEC_SIZE_ARRAY; do
		tst_netload -H $(tst_ipaddr rhost) -T dccp -n $p -N $p \
			-r $IPSEC_REQUESTS
	done
}

do_setup

do_test

tst_exit
