#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2016 Red Hat Inc.,  All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
# Author: Hangbin Liu <haliu@redhat.com>

TCID=${TCID:-icmp-uni-basic}
TST_TOTAL=1
TST_COUNT=1
TST_CLEANUP="tst_ipsec_cleanup"

. ipsec_lib.sh

tst_ipsec_setup

PING_MAX="$IPSEC_REQUESTS"

tst_resm TINFO "Sending ICMP messages"
tst_ping $(tst_iface) $(tst_ipaddr rhost) $IPSEC_SIZE_ARRAY

tst_exit
