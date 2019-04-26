#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2016 Red Hat Inc.,  All Rights Reserved.
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
# Author: Hangbin Liu <haliu@redhat.com>

# Authenticated encryption with associated data
AEALGO="rfc4106_128"
# Encryption algorithm
EALGO="des3_ede"
# Authentication algorithm
AALGO="sha1"
# Compression algorithm
CALGO="deflate"

IPSEC_REQUESTS="500"

ipsec_lib_usage()
{
	echo "l n     n is the number of test link when tests run"
	echo "m x     x is ipsec mode, could be transport / tunnel"
	echo "p x     x is ipsec protocol, could be ah / esp / comp"
	echo "s x     x is icmp message size array"
	echo "S n     n is IPsec SPI value"
	echo "k x     key for vti interface"
	echo "A x     Authenticated encryption with associated data algorithm"
	echo "e x     Encryption algorithm"
	echo "a x     Authentication algorithm"
	echo "c x     Compression algorithm"
	echo "r x     Num of requests, PING_MAX or netstress' '-r' opt"
}

ipsec_lib_parse_args()
{
	case "$1" in
	l) LINK_NUM=$2;;
	m) IPSEC_MODE=$2;;
	p) IPSEC_PROTO=$2;;
	s) TST_TEST_DATA="$2"; TST_TEST_DATA_IFS=":";;
	S) SPI=$2;;
	k) VTI_KEY=$2;;
	A) AEALGO=$2;;
	e) EALGO=$2;;
	a) AALGO=$2;;
	c) CALGO=$2;;
	r) IPSEC_REQUESTS="$2";;
	esac
}

ipsec_lib_setup()
{
	case $AEALGO in
	rfc4106_128|rfc4543_128) AEALGO_KEY=$(get_key 160);;
	rfc4106_192|rfc4543_192) AEALGO_KEY=$(get_key 224);;
	rfc4106_256|rfc4543_256) AEALGO_KEY=$(get_key 288);;
	rfc4309_128) AEALGO_KEY=$(get_key 152);;
	rfc4309_192) AEALGO_KEY=$(get_key 216);;
	rfc4309_256) AEALGO_KEY=$(get_key 280);;
	esac

	case $EALGO in
	des) EALGO_KEY=$(get_key 64);;
	des3_ede) EALGO_KEY=$(get_key 192);;
	cast5) EALGO_KEY=$(get_key 128);;
	blowfish) EALGO_KEY=$(get_key 448);;
	aes|twofish|camellia|serpent) EALGO_KEY=$(get_key 256);;
	*) tst_brk TBROK "unknown enc alg: $EALGO";;
	esac

	case $AALGO in
	sha1|rmd160) AALGO_KEY=$(get_key 160);;
	sha256) AALGO_KEY=$(get_key 256);;
	sha384) AALGO_KEY=$(get_key 384);;
	sha512) AALGO_KEY=$(get_key 512);;
	*) tst_brk TBROK "unknown auth alg: $AALGO";;
	esac

	SPI=${SPI:-1000}
	VTI_KEY=${VTI_KEY:-10}
	cleanup_vti=
	ALG=
	ALGR=

	if [ -n "$IPSEC_MODE" ]; then
		tst_net_run "tst_check_drivers xfrm_user" || \
			tst_brk TCONF "xfrm_user driver not available on lhost or rhost"
		cleanup_xfrm=1
	fi
}

TST_OPTS="l:m:p:s:S:k:A:e:a:c:r:"
TST_PARSE_ARGS=ipsec_lib_parse_args
TST_SETUP=${TST_SETUP:-ipsec_lib_setup}
TST_USAGE=ipsec_lib_usage
. tst_net.sh

get_key()
{
	local bits=$1
	local bytes=$(( $bits / 8))
	echo "0x$(hexdump -vn $bytes -e '1/1 "%02x"' /dev/urandom)"
}

tst_ipsec_setup()
{
	ipsec_lib_setup
	# Configure SAD/SPD
	if [ -n "$IPSEC_MODE" -a -n "$IPSEC_PROTO" ]; then
		tst_res TINFO "IPsec[$IPSEC_PROTO/$IPSEC_MODE]"
		tst_ipsec lhost $(tst_ipaddr) $(tst_ipaddr rhost)
		tst_ipsec rhost $(tst_ipaddr rhost) $(tst_ipaddr)
	fi
}

# tst_ipsec_cleanup: flush ipsec state and policy rules
tst_ipsec_cleanup()
{
	[ -z "$cleanup_xfrm" ] && return

	ip xfrm state flush
	ip xfrm policy flush
	tst_rhost_run -c "ip xfrm state flush && ip xfrm policy flush"

	if [ -n "$cleanup_vti" ]; then
		ip li del $cleanup_vti 2>/dev/null
		tst_rhost_run -c "ip li del $cleanup_vti 2>/dev/null"
	fi
}

ipsec_set_algoline()
{
	case $IPSEC_PROTO in
	ah)
		ALG='auth hmac('$AALGO') '$AALGO_KEY
		ALGR='auth hmac\('$AALGO'\) '$AALGO_KEY
		;;
	esp)
		ALG="enc $EALGO $EALGO_KEY auth "'hmac('$AALGO') '$AALGO_KEY
		ALGR="enc $EALGO $EALGO_KEY auth "'hmac\('$AALGO'\) '$AALGO_KEY
		;;
	esp_aead)
		case $AEALGO in
		rfc4106_128|rfc4106_192|rfc4106_256)
			ALG="aead "'rfc4106(gcm(aes))'" $AEALGO_KEY 128"
			ALGR="aead "'rfc4106\(gcm\(aes\)\)'" $AEALGO_KEY 128"
			;;
		rfc4309_128|rfc4309_192|rfc4309_256)
			ALG="aead "'rfc4309(ccm(aes))'" $AEALGO_KEY 128"
			ALGR="aead "'rfc4309\(ccm\(aes\)\)'" $AEALGO_KEY 128"
			;;
		rfc4543_128|rfc4543_192|rfc4543_256)
			ALG="aead "'rfc4543(gcm(aes))'" $AEALGO_KEY 128"
			ALGR="aead "'rfc4543\(gcm\(aes\)\)'" $AEALGO_KEY 128"
			;;
		esac
		;;
	comp)
		ALG="comp $CALGO"
		ALGR=$ALG
		;;
	*)
		tst_brk TCONF "tst_ipsec protocol mismatch"
		;;
	esac
}

# tst_ipsec target src_addr dst_addr: config ipsec
#
# target: target of the configuration host ( lhost / rhost )
# src_addr: source IP address
# dst_addr: destination IP address
tst_ipsec()
{
	if [ $# -ne 3 ]; then
		tst_brk TCONF "tst_ipsec parameter mismatch"
	fi

	local target=$1
	local src=$2
	local dst=$3
	local mode=$IPSEC_MODE
	local p="proto $IPSEC_PROTO"
	[ "$IPSEC_PROTO" = "esp_aead" ] && p="proto esp"

	ipsec_set_algoline

	if [ $target = lhost ]; then
		local spi_1="0x$SPI"
		local spi_2="0x$(( $SPI + 1 ))"
		TST_RTNL_CHK ip xfrm state add src $src dst $dst spi $spi_1 \
			$p $ALG mode $mode sel src $src dst $dst
		ROD ip xfrm state add src $dst dst $src spi $spi_2 \
			$p $ALG mode $mode sel src $dst dst $src

		ROD ip xfrm policy add src $src dst $dst dir out tmpl src $src \
			dst $dst $p mode $mode
		ROD ip xfrm policy add src $dst dst $src dir in tmpl src $dst \
			dst $src $p mode $mode level use
	elif [ $target = rhost ]; then
		local spi_1="0x$(( $SPI + 1 ))"
		local spi_2="0x$SPI"
		tst_rhost_run -s -c "ip xfrm state add src $src dst $dst \
			spi $spi_1 $p $ALGR mode $mode sel src $src dst $dst"
		tst_rhost_run -s -c "ip xfrm state add src $dst dst $src \
			spi $spi_2 $p $ALGR mode $mode sel src $dst dst $src"

		tst_rhost_run -s -c "ip xfrm policy add src $src dst $dst \
			dir out tmpl src $src dst $dst $p mode $mode"
		tst_rhost_run -s -c "ip xfrm policy add src $dst dst $src dir \
			in tmpl src $dst dst $src $p mode $mode level use"
	fi
}

# tst_ipsec_vti target src_addr dst_addr vti_name
#
# target: target of the configuration host ( lhost / rhost )
# src_addr: source IP address
# dst_addr: destination IP address
# vti_name: name of vti interface
tst_ipsec_vti()
{
	if [ $# -ne 4 ]; then
		tst_brk TCONF "tst_ipsec_vti parameter mismatch"
	fi

	local target=$1
	local src=$2
	local dst=$3
	local vti=$4
	local m="mode $IPSEC_MODE"
	local p="proto $IPSEC_PROTO"
	[ "$IPSEC_PROTO" = "esp_aead" ] && p="proto esp"

	local key="key $VTI_KEY"
	local mrk="mark $VTI_KEY"
	local type="type vti$TST_IPV6"
	local d="dev $(tst_iface)"
	local rd="dev $(tst_iface rhost)"

	ip li add type vti help 2>&1 | grep -q vti || \
		tst_brk TCONF "iproute doesn't support 'vti'"

	ipsec_set_algoline

	local o_dir="src $src dst $dst"
	local i_dir="src $dst dst $src"
	local ipx="ip -$TST_IPVER xf"

	cleanup_vti=$vti

	if [ $target = lhost ]; then
		TST_RTNL_CHK ip li add $vti $type local $src remote $dst $key $d
		ROD ip li set $vti up

		local spi_1="spi 0x$SPI"
		local spi_2="spi 0x$(( $SPI + 1 ))"
		TST_RTNL_CHK $ipx st add $o_dir $p $spi_1 $ALG $m
		ROD $ipx st add $i_dir $p $spi_2 $ALG $m
		ROD $ipx po add dir out tmpl $o_dir $p $m $mrk
		ROD $ipx po add dir in tmpl $i_dir $p $m $mrk
	elif [ $target = rhost ]; then
		tst_rhost_run -s -c \
			"ip li add $vti $type local $src remote $dst $key $rd"
		tst_rhost_run -s -c "ip li set $vti up"

		local spi_1="spi 0x$(( $SPI + 1 ))"
		local spi_2="spi 0x$SPI"
		tst_rhost_run -s -c "$ipx st add $o_dir $p $spi_1 $ALGR $m"
		tst_rhost_run -s -c "$ipx st add $i_dir $p $spi_2 $ALGR $m"
		tst_rhost_run -s -c "$ipx po add dir out tmpl $o_dir $p $m $mrk"
		tst_rhost_run -s -c "$ipx po add dir in tmpl $i_dir $p $m $mrk"
	fi
}

# Setup vti/vti6 interface for IPsec tunneling
# The function sets variables:
#  * tst_vti - vti interface name,
#  * ip_loc_tun - local IP address on vti interface
#  * ip_rmt_tun - remote IP address
tst_ipsec_setup_vti()
{
	ipsec_lib_setup

	if_loc=$(tst_iface)
	if_rmt=$(tst_iface rhost)

	ip_loc=$(tst_ipaddr)
	ip_rmt=$(tst_ipaddr rhost)

	tst_vti="ltp_vti0"

	tst_res TINFO "Test vti$TST_IPV6 + IPsec[$IPSEC_PROTO/$IPSEC_MODE]"

	tst_ipsec_vti lhost $ip_loc $ip_rmt $tst_vti
	tst_ipsec_vti rhost $ip_rmt $ip_loc $tst_vti

	local mask=
	if [ "$TST_IPV6" ]; then
		ip_loc_tun="${IPV6_NET32_UNUSED}::1";
		ip_rmt_tun="${IPV6_NET32_UNUSED}::2";
		mask=64
		ROD ip -6 route add ${IPV6_NET32_UNUSED}::/$mask dev $tst_vti
	else
		ip_loc_tun="${IPV4_NET16_UNUSED}.1.1";
		ip_rmt_tun="${IPV4_NET16_UNUSED}.1.2";
		mask=30
		ROD ip route add ${IPV4_NET16_UNUSED}.1.0/$mask dev $tst_vti
	fi

	tst_res TINFO "Add IPs to vti tunnel, " \
		       "loc: $ip_loc_tun/$mask, rmt: $ip_rmt_tun/$mask"

	ROD ip a add $ip_loc_tun/$mask dev $tst_vti nodad
	tst_rhost_run -s -c "ip a add $ip_rmt_tun/$mask dev $tst_vti"
}
