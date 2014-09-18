#!/bin/sh

# Copyright (c) 2014 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2006
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

. test_net.sh

TST_IPV6=6

tst_resm TINFO "Test duration is $NS_DURATION [sec]"

tst_restore_ipaddr
tst_restore_ipaddr rhost

lhost_addr=$(tst_ipaddr)
rhost_addr=$(tst_ipaddr rhost)

tst_rhost_run -s -c \
	"check_icmpv6_connectivity $(tst_iface rhost) $lhost_addr"
