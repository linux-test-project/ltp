#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2020 Oracle and/or its affiliates. All Rights Reserved.

TST_CLEANUP=cleanup
TST_TESTFUNC=test1
TST_SETUP=wireguard_lib_setup

IPSEC_MODE="tunnel"
IPSEC_PROTO="esp_aead"
AEALGO="rfc4106_256"
EALGO="aes"
AALGO="sha256"

cleanup()
{
	wireguard_lib_cleanup
	tst_ipsec_cleanup
}

test1()
{
	local wgaddr
	local clients_num="$TST_NETLOAD_CLN_NUMBER"

	# Enforce multi-threading test, at least with 10 TCP clients
	[ $clients_num -lt 10 ] && clients_num=10

	tst_res TINFO "test wireguard"

	[ -n "$TST_IPV6" ] && wgaddr="$ip6_virt_remote" || wgaddr="$ip_virt_remote"
	tst_netload -H $wgaddr -a $clients_num -D ltp_v0
	local time_wg=$(cat tst_netload.res)
	wireguard_lib_cleanup

	tst_res TINFO "test IPSec $IPSEC_MODE/$IPSEC_PROTO $EALGO"
	tst_ipsec_setup_vti
	tst_netload -H $ip_rmt_tun -a $clients_num -D $tst_vti
	local time_ipsec=$(cat tst_netload.res)
	tst_ipsec_cleanup

	tst_netload_compare $time_ipsec $time_wg -100
}

. ipsec_lib.sh
. wireguard_lib.sh
tst_run
