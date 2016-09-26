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

while getopts "hl:m:p:s:S:6" opt; do
	case "$opt" in
	h)
		echo "Usage:"
		echo "h        help"
		echo "l n      n is the number of test link when tests run"
		echo "m x      x is ipsec mode, could be transport / tunnel"
		echo "p x      x is ipsec protocol, could be ah / esp / ipcomp"
		echo "s x      x is icmp messge size array"
		echo "S n      n is IPsec SPI value"
		echo "6        run over IPv6"
		exit 0
	;;
	l) LINK_NUM=$OPTARG ;;
	m) IPSEC_MODE=$OPTARG ;;
	p) IPSEC_PROTO=$OPTARG ;;
	s) ICMP_SIZE_ARRAY=$OPTARG ;;
	S) SPI=$OPTARG ;;
	6) # skip, test_net library already processed it
	;;
	*) tst_brkm TBROK "unknown option: $opt" ;;
	esac
done

SPI=${SPI:-1000}

# tst_ipsec_cleanup: flush ipsec state and policy rules
tst_ipsec_cleanup()
{
	ip xfrm state flush
	ip xfrm policy flush
	tst_rhost_run -c "ip xfrm state flush && ip xfrm policy flush"
}

tst_check_cmds hexdump

# Encryption algorithm
EALGO="des3_ede"
EALGO_KEY=0x$(printf _I_want_to_have_chicken_ | hexdump -ve '/1 "%x"')

# Authentication algorithm
AALGO="sha1"
AALGO_KEY=0x$(printf beef_fish_pork_salad | hexdump -ve '/1 "%x"')

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

	# Compression algorithm
	local CALGO="deflate"
	# Algorithm options for each protocol
	local algo_line=
	case $IPSEC_PROTO in
	ah)
		algo_line="auth $AALGO $AALGO_KEY"
		proto="ah"
		;;
	esp)
		algo_line="enc $EALGO $EALGO_KEY auth $AALGO $AALGO_KEY"
		proto="esp"
		;;
	comp)
		algo_line="comp $CALGO"
		proto="comp"
		;;
	*)
		tst_brkm TCONF "tst_ipsec protocol mismatch"
		;;
	esac

	local mode=$IPSEC_MODE

	if [ $target = lhost ]; then
		local spi_1="0x$SPI"
		local spi_2="0x$(( $SPI + 1 ))"
		ROD ip xfrm state add src $src dst $dst spi $spi_1 \
			proto $proto $algo_line mode $mode sel src $src dst $dst
		ROD ip xfrm state add src $dst dst $src spi $spi_2 \
			proto $proto $algo_line mode $mode sel src $dst dst $src

		ROD ip xfrm policy add src $src dst $dst dir out tmpl src $src \
			dst $dst proto $proto mode $mode
		ROD ip xfrm policy add src $dst dst $src dir in tmpl src $dst \
			dst $src proto $proto mode $mode level use
	elif [ $target = rhost ]; then
		local spi_1="0x$(( $SPI + 1 ))"
		local spi_2="0x$SPI"
		tst_rhost_run -s -c "ip xfrm state add src $src dst $dst \
			spi $spi_1 proto $proto $algo_line mode $mode sel \
			src $src dst $dst"
		tst_rhost_run -s -c "ip xfrm state add src $dst dst $src \
			spi $spi_2 proto $proto $algo_line mode $mode sel \
			src $dst dst $src"

		tst_rhost_run -s -c "ip xfrm policy add src $src dst $dst \
			dir out tmpl src $src dst $dst proto $proto mode $mode"
		tst_rhost_run -s -c "ip xfrm policy add src $dst dst $src dir \
			in tmpl src $dst dst $src proto $proto \
			mode $mode level use"
	fi
}
