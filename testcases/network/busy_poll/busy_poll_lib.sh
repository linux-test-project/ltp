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

if tst_kvcmp -lt "3.11"; then
	tst_brkm TCONF "test must be run with kernel 3.11 or newer"
fi

if [ ! -f "/proc/sys/net/core/busy_read" -a \
     ! -f "/proc/sys/net/core/busy_poll" ]; then
	tst_brkm TCONF "busy poll not configured, CONFIG_NET_RX_BUSY_POLL"
fi

tst_check_cmds pkill sysctl ethtool

if tst_kvcmp -lt "4.5"; then
	ethtool --show-features $(tst_iface) | \
		grep -q 'busy-poll.*on' || \
		tst_brkm TCONF "busy poll not supported by driver"
else
	drvs="bnx2x|bnxt|cxgb4|enic|benet|ixgbe|ixgbevf|mlx4|mlx5|myri10ge|sfc|virtio"
	ethtool -i $(tst_iface) | grep -qE "driver: ($drvs)" || \
		tst_brkm TCONF "busy poll not supported"
fi

tst_require_root
