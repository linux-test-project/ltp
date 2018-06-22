#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016-2018 Oracle and/or its affiliates. All Rights Reserved.

# for more stable results set to a single thread
TST_NETLOAD_CLN_NUMBER=1

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
