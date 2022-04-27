#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2022
# Copyright (c) 2020 Oracle and/or its affiliates. All Rights Reserved.

TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="$TST_NEEDS_CMDS wg"
TST_TESTFUNC=${TST_TESTFUNC:-virt_netperf_msg_sizes}
TST_SETUP=${TST_SETUP:-wireguard_lib_setup}
TST_CLEANUP=${TST_CLEANUP:-wireguard_lib_cleanup}
TST_NEEDS_DRIVERS="wireguard"
VIRT_PERF_THRESHOLD_MIN=${VIRT_PERF_THRESHOLD_MIN:-200}

virt_type="wireguard"

# Usage: wireguard_lib_setup [TYPE]
# TYPE: [ default | invalid_allowed_ips | invalid_pub_keys ]
wireguard_lib_setup()
{
	local type="${1:-default}"
	local pub_key0="$(wg genkey | tee wg0.key | wg pubkey)"
	local pub_key1="$(wg genkey | tee wg1.key | wg pubkey)"

	local port_loc="$(tst_get_unused_port ipv${TST_IPVER} dgram)"
	local port_rmt=$(tst_rhost_run -c "tst_get_unused_port ipv${TST_IPVER} dgram")

	# copy private key to remote host
	tst_rhost_run -s -c "echo '$(cat wg1.key)' > wg1.key"

	tst_res TINFO "setup wireguard UDPv${TST_IPVER} tunnel, port $port_loc/$port_rmt"
	tst_res TINFO "lhost[$(tst_ipaddr)] <-> rhost[$(tst_ipaddr rhost)]"

	virt_setup

	local ka_opt="persistent-keepalive 1"
	local allow_ip_loc="${ip_virt_local}/32,${ip6_virt_local}/128"
	local allow_ip_rmt="${ip_virt_remote}/32,${ip6_virt_remote}/128"

	case $type in
	invalid_allowed_ips)
		allow_ip_loc="${ip_virt_remote}/32,${ip6_virt_remote}/128"
		allow_ip_rmt="${ip_virt_local}/32,${ip6_virt_local}/128"
		tst_res TINFO "Allowed IPs are source IPs only"
		;;
	invalid_pub_keys)
		pub_key0="$(wg genkey | wg pubkey)"
		tst_res TINFO "Invalid peer public key of lhost"
		;;
	esac

	ROD wg set ltp_v0 listen-port $port_loc private-key wg0.key
	ROD wg set ltp_v0 peer $pub_key1 endpoint \
		$(tst_ipaddr rhost):$port_rmt $ka_opt \
		allowed-ips $allow_ip_rmt

	tst_rhost_run -s -c \
		"wg set ltp_v0 listen-port $port_rmt private-key wg1.key"
	tst_rhost_run -s -c "wg set ltp_v0 peer $pub_key0 \
		endpoint $(tst_ipaddr):$port_loc $ka_opt \
		allowed-ips $allow_ip_loc"
}

wireguard_lib_cleanup()
{
	virt_cleanup
}

. virt_lib.sh
