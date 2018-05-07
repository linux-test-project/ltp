#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Test-case: setup two MACsec drivers and run TCP traffic over them
# with enabled frame encryption and replay protection, compare
# performance with similar IPsec configuration on master interface.

TCID=macsec02
TST_TOTAL=16
TST_NEEDS_TMPDIR=1

virt_type="macsec"
VIRT_PERF_THRESHOLD=${VIRT_PERF_THRESHOLD:-100}

. ipsec_lib.sh
. virt_lib.sh

cleanup()
{
	virt_cleanup
	tst_ipsec_cleanup
}
TST_CLEANUP="cleanup"

IPSEC_MODE=transport
IPSEC_PROTO=esp_aead
EALGO=aes
tst_resm TINFO "setup IPsec $IPSEC_MODE/$IPSEC_PROTO $EALGO"
tst_ipsec lhost $(tst_ipaddr) $(tst_ipaddr rhost)
tst_ipsec rhost $(tst_ipaddr rhost) $(tst_ipaddr)

virt_macsec_setup "replay on window 300 encrypt on protect on"
virt_netperf_msg_sizes

tst_exit
