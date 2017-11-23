#!/bin/sh
# Copyright (c) 2016 Red Hat Inc.,  All Rights Reserved.
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
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#
# Author: Hangbin Liu <haliu@redhat.com>
#
#######################################################################

. test_net.sh

# Authenticated encryption with associated data
AEALGO="rfc4106_128"
# Encryption algorithm
EALGO="des3_ede"
# Authentication algorithm
AALGO="sha1"
# Compression algorithm
CALGO="deflate"

IPSEC_REQUESTS="500"
IPSEC_SIZE_ARRAY="${IPSEC_SIZE_ARRAY:-10 100 1000 2000 10000 65000}"

while getopts "hl:m:p:s:S:k:A:e:a:c:r:6" opt; do
	case "$opt" in
	h)
		echo "Usage:"
		echo "h        help"
		echo "l n      n is the number of test link when tests run"
		echo "m x      x is ipsec mode, could be transport / tunnel"
		echo "p x      x is ipsec protocol, could be ah / esp / comp"
		echo "s x      x is icmp messge size array"
		echo "S n      n is IPsec SPI value"
		echo "k x      key for vti interface"
		echo "A x      Authenticated encryption with associated data algorithm"
		echo "e x      Encryption algorithm"
		echo "a x      Authentication algorithm"
		echo "c x      Compression algorithm"
		echo "r x      Num of requests, PING_MAX or netstress' '-r' opt"
		echo "6        run over IPv6"
		exit 0
	;;
	l) LINK_NUM=$OPTARG ;;
	m) IPSEC_MODE=$OPTARG ;;
	p) IPSEC_PROTO=$OPTARG ;;
	s) IPSEC_SIZE_ARRAY="$OPTARG" ;;
	S) SPI=$OPTARG ;;
	k) VTI_KEY=$OPTARG ;;
	A) AEALGO=$OPTARG ;;
	e) EALGO=$OPTARG ;;
	a) AALGO=$OPTARG ;;
	c) CALGO=$OPTARG ;;
	r) IPSEC_REQUESTS="$OPTARG" ;;
	6) # skip, test_net library already processed it
	;;
	*) tst_brkm TBROK "unknown option: $opt" ;;
	esac
done

get_key()
{
	local bits=$1
	local xdg_num=$(( $bits / 4 ))
	echo "0x$(tr -dc "[:xdigit:]" < /dev/urandom | head -c$xdg_num)"
}

case $AEALGO in
rfc4106_128|rfc4543_128) AEALGO_KEY=$(get_key 160) ;;
rfc4106_192|rfc4543_192) AEALGO_KEY=$(get_key 224) ;;
rfc4106_256|rfc4543_256) AEALGO_KEY=$(get_key 288) ;;
rfc4309_128) AEALGO_KEY=$(get_key 152) ;;
rfc4309_192) AEALGO_KEY=$(get_key 216) ;;
rfc4309_256) AEALGO_KEY=$(get_key 280) ;;
esac

case $EALGO in
des) EALGO_KEY=$(get_key 64) ;;
des3_ede) EALGO_KEY=$(get_key 192) ;;
cast5) EALGO_KEY=$(get_key 128) ;;
blowfish) EALGO_KEY=$(get_key 448) ;;
aes|twofish|camellia|serpent) EALGO_KEY=$(get_key 256) ;;
*) tst_brkm TBROK "unknown enc alg: $EALGO" ;;
esac

case $AALGO in
sha1|rmd160) AALGO_KEY=$(get_key 160) ;;
sha256) AALGO_KEY=$(get_key 256) ;;
sha384) AALGO_KEY=$(get_key 384) ;;
sha512) AALGO_KEY=$(get_key 512) ;;
*) tst_brkm TBROK "unknown auth alg: $AALGO" ;;
esac

SPI=${SPI:-1000}
VTI_KEY=${VTI_KEY:-10}
cleanup_vti=
ALG=
ALGR=

# tst_ipsec_cleanup: flush ipsec state and policy rules
tst_ipsec_cleanup()
{
	ip xfrm state flush
	ip xfrm policy flush
	tst_rhost_run -c "ip xfrm state flush && ip xfrm policy flush"

	if [ -n "$cleanup_vti" ]; then
		ip li del $cleanup_vti 2>/dev/null
		tst_rhost_run -c "ip li del $cleanup_vti 2>/dev/null"
	fi

	[ "$TST_NEEDS_TMPDIR" = 1 ] && tst_rmdir
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
		tst_brkm TCONF "tst_ipsec protocol mismatch"
		;;
	esac
}

ipsec_try()
{
	local output="$($@ 2>&1 || echo 'TERR')"

	if echo "$output" | grep -q "TERR"; then
		echo "$output" | grep -q \
			'RTNETLINK answers: Function not implemented' && \
			tst_brkm TCONF "'$@': not implemented"
		echo "$output" | grep -q \
			'RTNETLINK answers: Operation not supported' && \
			tst_brkm TCONF "'$@': not supported (maybe missing 'ip${TST_IPV6}_vti' kernel module)"
		tst_brkm TBROK "$@ failed: $output"
	fi
}

# tst_ipsec target src_addr dst_addr: config ipsec
#
# target: target of the configuration host ( lhost / rhost )
# src_addr: source IP address
# dst_addr: destination IP address
tst_ipsec()
{
	if [ $# -ne 3 ]; then
		tst_brkm TCONF "tst_ipsec parameter mismatch"
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
		ipsec_try ip xfrm state add src $src dst $dst spi $spi_1 \
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
		tst_brkm TCONF "tst_ipsec_vti parameter mismatch"
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
		tst_brkm TCONF "iproute doesn't support 'vti'"

	ipsec_set_algoline

	local o_dir="src $src dst $dst"
	local i_dir="src $dst dst $src"
	local ipx="ip -${TST_IPV6:-4} xf"

	cleanup_vti=$vti

	if [ $target = lhost ]; then
		ipsec_try ip li add $vti $type local $src remote $dst $key $d
		ROD ip li set $vti up

		local spi_1="spi 0x$SPI"
		local spi_2="spi 0x$(( $SPI + 1 ))"
		ipsec_try $ipx st add $o_dir $p $spi_1 $ALG $m
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
	if_loc=$(tst_iface)
	if_rmt=$(tst_iface rhost)

	ip_loc=$(tst_ipaddr)
	ip_rmt=$(tst_ipaddr rhost)

	tst_vti="ltp_vti0"

	tst_resm TINFO "Test vti$TST_IPV6 + IPsec[$IPSEC_PROTO/$IPSEC_MODE]"

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

	tst_resm TINFO "Add IPs to vti tunnel, " \
		       "loc: $ip_loc_tun/$mask, rmt: $ip_rmt_tun/$mask"

	ROD ip a add $ip_loc_tun/$mask dev $tst_vti nodad
	tst_rhost_run -s -c "ip a add $ip_rmt_tun/$mask dev $tst_vti"
}
