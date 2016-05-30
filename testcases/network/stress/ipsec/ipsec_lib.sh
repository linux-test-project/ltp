#!/bin/sh
# Copyright (c) 2016 Red Hat Inc.,  All Rights Reserved.
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

# tst_ipsec_cleanup: flush ipsec state and policy rules
tst_ipsec_cleanup()
{
	ip xfrm state flush
	ip xfrm policy flush
	tst_rhost_run -c "ip xfrm state flush && ip xfrm policy flush"
}

# tst_ipsec target protocol mode spi src_addr dst_addr: config ipsec with
# supplied protocol and mode.
#
# target: target of the configuration host ( lhost / rhost )
# protocol: ah / esp / ipcomp
# mode: transport / tunnel
# spi: the first spi value
# src_addr: source IP address
# dst_addr: destination IP address
tst_ipsec()
{
	if [ $# -ne 6 ]; then
		tst_brkm TCONF "tst_ipsec parameter mismatch"
	fi
	tst_check_cmds hexdump

	local target=$1
	local protocol=$2
	local mode=$3
	local spi=$4
	local src=$5
	local dst=$6

	# Encryption algorithm
	local EALGO="des3_ede"
	local EALGO_KEY=0x$(printf _I_want_to_have_chicken_ | \
			    hexdump -ve '/1 "%x"')

	# Authentication algorithm
	local AALGO="sha1"
	local AALGO_KEY=0x$(printf beef_fish_pork_salad | \
			    hexdump -ve '/1 "%x"')

	# Compression algorithm
	local CALGO="deflate"
	# Algorithm options for each protocol
	local algo_line=
	local proto=
	case $protocol in
	ah)
		algo_line="auth $AALGO $AALGO_KEY"
		proto="ah"
		;;
	esp)
		algo_line="enc $EALGO $EALGO_KEY auth $AALGO $AALGO_KEY"
		proto="esp"
		;;
	ipcomp)
		algo_line="comp $CALGO"
		proto="comp"
		;;
	*)
		tst_brkm TCONF "tst_ipsec protocol mismatch"
		;;
	esac

	if [ $target = lhost ]; then
		local spi_1="0x$spi"
		local spi_2="0x$(( $spi + 1 ))"
		ROD ip xfrm state add src $src dst $dst spi $spi_1 \
			proto $proto $algo_line mode $mode sel src $src dst $dst
		ROD ip xfrm state add src $dst dst $src spi $spi_2 \
			proto $proto $algo_line mode $mode sel src $dst dst $src

		ROD ip xfrm policy add src $src dst $dst dir out tmpl src $src \
			dst $dst proto $proto mode $mode
		ROD ip xfrm policy add src $dst dst $src dir in tmpl src $dst \
			dst $src proto $proto mode $mode level use
	elif [ $target = rhost ]; then
		local spi_1="0x$(( $spi + 1 ))"
		local spi_2="0x$spi"
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
