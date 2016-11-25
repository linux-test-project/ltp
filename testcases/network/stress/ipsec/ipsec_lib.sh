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

# Encryption algorithm
EALGO="des3_ede"
# Authentication algorithm
AALGO="sha1"
# Compression algorithm
CALGO="deflate"

while getopts "hl:m:p:s:S:k:e:a:c:6" opt; do
	case "$opt" in
	h)
		echo "Usage:"
		echo "h        help"
		echo "l n      n is the number of test link when tests run"
		echo "m x      x is ipsec mode, could be transport / tunnel"
		echo "p x      x is ipsec protocol, could be ah / esp / ipcomp"
		echo "s x      x is icmp messge size array"
		echo "S n      n is IPsec SPI value"
		echo "k x      key for vti interface"
		echo "e x      Encryption algorithm"
		echo "a x      Authentication algorithm"
		echo "c x      Compression algorithm"
		echo "6        run over IPv6"
		exit 0
	;;
	l) LINK_NUM=$OPTARG ;;
	m) IPSEC_MODE=$OPTARG ;;
	p) IPSEC_PROTO=$OPTARG ;;
	s) ICMP_SIZE_ARRAY=$OPTARG ;;
	S) SPI=$OPTARG ;;
	k) VTI_KEY=$OPTARG ;;
	e) EALGO=$OPTARG ;;
	a) AALGO=$OPTARG ;;
	c) CALGO=$OPTARG ;;
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
	comp)
		ALG="comp $CALGO"
		ALGR=$ALG
		;;
	*)
		tst_brkm TCONF "tst_ipsec protocol mismatch"
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
		tst_brkm TCONF "tst_ipsec parameter mismatch"
	fi

	local target=$1
	local src=$2
	local dst=$3
	local mode=$IPSEC_MODE
	local proto=$IPSEC_PROTO

	ipsec_set_algoline

	if [ $target = lhost ]; then
		local spi_1="0x$SPI"
		local spi_2="0x$(( $SPI + 1 ))"
		ROD ip xfrm state add src $src dst $dst spi $spi_1 \
			proto $proto $ALG mode $mode sel src $src dst $dst
		ROD ip xfrm state add src $dst dst $src spi $spi_2 \
			proto $proto $ALG mode $mode sel src $dst dst $src

		ROD ip xfrm policy add src $src dst $dst dir out tmpl src $src \
			dst $dst proto $proto mode $mode
		ROD ip xfrm policy add src $dst dst $src dir in tmpl src $dst \
			dst $src proto $proto mode $mode level use
	elif [ $target = rhost ]; then
		local spi_1="0x$(( $SPI + 1 ))"
		local spi_2="0x$SPI"
		tst_rhost_run -s -c "ip xfrm state add src $src dst $dst \
			spi $spi_1 proto $proto $ALGR mode $mode sel \
			src $src dst $dst"
		tst_rhost_run -s -c "ip xfrm state add src $dst dst $src \
			spi $spi_2 proto $proto $ALGR mode $mode sel \
			src $dst dst $src"

		tst_rhost_run -s -c "ip xfrm policy add src $src dst $dst \
			dir out tmpl src $src dst $dst proto $proto mode $mode"
		tst_rhost_run -s -c "ip xfrm policy add src $dst dst $src dir \
			in tmpl src $dst dst $src proto $proto \
			mode $mode level use"
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
	local key="key $VTI_KEY"
	local mrk="mark $VTI_KEY"
	local type="type vti$TST_IPV6"

	ip li add type vti help 2>&1 | grep -q vti || \
		tst_brkm TCONF "iproute doesn't support 'vti'"

	ipsec_set_algoline

	local o_dir="src $src dst $dst"
	local i_dir="src $dst dst $src"
	local ipx="ip -${TST_IPV6:-4} xf"

	cleanup_vti=$vti

	if [ $target = lhost ]; then
		ROD ip li add $vti $type local $src remote $dst $key
		ROD ip li set $vti up

		local spi_1="spi 0x$SPI"
		local spi_2="spi 0x$(( $SPI + 1 ))"
		ROD $ipx st add $o_dir $p $spi_1 $ALG $m
		ROD $ipx st add $i_dir $p $spi_2 $ALG $m
		ROD $ipx po add dir out tmpl $o_dir $p $m $mrk
		ROD $ipx po add dir in tmpl $i_dir $p $m $mrk
	elif [ $target = rhost ]; then
		tst_rhost_run -s -c \
			"ip li add $vti $type local $src remote $dst $key"
		tst_rhost_run -s -c "ip li set $vti up"

		local spi_1="spi 0x$(( $SPI + 1 ))"
		local spi_2="spi 0x$SPI"
		tst_rhost_run -s -c "$ipx st add $o_dir $p $spi_1 $ALGR $m"
		tst_rhost_run -s -c "$ipx st add $i_dir $p $spi_2 $ALGR $m"
		tst_rhost_run -s -c "$ipx po add dir out tmpl $o_dir $p $m $mrk"
		tst_rhost_run -s -c "$ipx po add dir in tmpl $i_dir $p $m $mrk"
	fi
}
