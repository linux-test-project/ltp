#!/bin/sh
# Copyright (c) 2016 Red Hat Inc.,  All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
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
################################################################################
TCID=${TCID:-icmp-uni-basic}
TST_TOTAL=1
TST_COUNT=1
TST_CLEANUP="tst_ipsec_cleanup"

. ipsec_lib.sh

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
LINK_NUM=${LINK_NUM:-0}
DO_IPSEC=${DO_IPSEC:-false}
ICMP_SIZE_ARRAY=${ICMP_SIZE_ARRAY:-"10 100 1000 10000 65507"}
[ -n "$IPSEC_MODE" -a -n "$IPSEC_PROTO" ] && DO_IPSEC=true || DO_IPSEC=false

# Test description
tst_resm TINFO "Sending ICMP messages with the following conditions"
tst_resm TINFO "- Version of IP is IPv${TST_IPV6:-4}"
tst_resm TINFO "- Size of packets are ( $ICMP_SIZE_ARRAY )"

if $DO_IPSEC; then
	case $IPSEC_PROTO in
	ah)	tst_resm TINFO "- IPsec [ AH / $IPSEC_MODE ]" ;;
	esp)	tst_resm TINFO "- IPsec [ ESP / $IPSEC_MODE ]" ;;
	ipcomp)	tst_resm TINFO "- IPcomp [ $IPSEC_MODE ]" ;;
	esac
fi

# name of interface of the local/remote host
lhost_ifname=$(tst_iface lhost $LINK_NUM)
rhost_ifname=$(tst_iface rhost $LINK_NUM)

lhost_addr=$(tst_ipaddr)
rhost_addr=$(tst_ipaddr rhost)

# Configure SAD/SPD
if $DO_IPSEC ; then
	tst_ipsec lhost $IPSEC_PROTO $IPSEC_MODE $SPI $lhost_addr $rhost_addr
	tst_ipsec rhost $IPSEC_PROTO $IPSEC_MODE $SPI $rhost_addr $lhost_addr
fi

tst_ping $lhost_ifname $rhost_addr $ICMP_SIZE_ARRAY
if [ $? -ne 0 ]; then
	tst_resm TFAIL "Checked IPv${TST_IPV6:-4} $IPSEC_PROTO $IPSEC_MODE"
else
	tst_resm TPASS "Checked IPv${TST_IPV6:-4} $IPSEC_PROTO $IPSEC_MODE"
fi

tst_exit
