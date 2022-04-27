#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2014-2017 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

virt_type="macsec"
VIRT_PERF_THRESHOLD_MIN=100
IPSEC_MODE="transport"

TST_NEEDS_TMPDIR=1
TST_TESTFUNC=virt_netperf_msg_sizes
TST_SETUP="${TST_SETUP:-macsec_lib_setup}"
TST_CLEANUP="${TST_CLEANUP:-macsec_lib_cleanup}"
TST_NEEDS_DRIVERS="macsec"

# MACSEC_LIB_SETUP:
# [ cipher { default | gcm-aes-128 } ] [ encrypt { on | off } ]
# [ protect { on | off } ] [ replay { on | off } ] [ window WINDOW ]
# [ validate { strict | check | disabled } ]
macsec_lib_setup()
{
	local keyid0="01"
	local keyid1="02"
	local sa=0
	local h0=$(tst_hwaddr)
	local h1=$(tst_hwaddr rhost)
	local cmd="ip macsec add ltp_v0"
	local key0="01234567890123456789012345678901"
	local key1="98765432109876543210987612343434"

	ipsec_lib_setup

	tst_res TINFO "setup IPsec $IPSEC_MODE/$IPSEC_PROTO $EALGO"
	tst_ipsec lhost $(tst_ipaddr) $(tst_ipaddr rhost)
	tst_ipsec rhost $(tst_ipaddr rhost) $(tst_ipaddr)

	virt_setup "icvlen 16 encodingsa $sa $MACSEC_LIB_SETUP"

	ROD $cmd tx sa $sa pn 100 on key $keyid0 $key0
	ROD $cmd rx address $h1 port 1
	ROD $cmd rx address $h1 port 1 sa $sa pn 100 on key $keyid1 $key1

	tst_rhost_run -s -c "$cmd tx sa $sa pn 100 on key $keyid1 $key1"
	tst_rhost_run -s -c "$cmd rx address $h0 port 1"
	tst_rhost_run -s -c \
		"$cmd rx address $h0 port 1 sa $sa pn 100 on key $keyid0 $key0"
}

macsec_lib_cleanup()
{
	virt_cleanup
	tst_ipsec_cleanup
}

. ipsec_lib.sh
. virt_lib.sh
