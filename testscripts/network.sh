#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2013-2019

cd $(dirname $0)
export LTPROOT=${LTPROOT:-"$PWD"}
echo $LTPROOT | grep -q testscripts
if [ $? -eq 0 ]; then
	cd ..
	export LTPROOT=${PWD}
fi

export TMPDIR=/tmp/netpan-$$
mkdir -p $TMPDIR
CMDFILE=${TMPDIR}/network.tests
VERBOSE="no"
NO_KMSG=
QUIET_MODE=
TEST_CASES=

export PATH="${PATH}:${LTPROOT}/testcases/bin"

usage()
{
	echo "Usage: $0 OPTIONS"
	echo "  -6    IPv6 tests"
	echo "  -m    multicast tests"
	echo "  -n    NFS tests"
	echo "  -s    SCTP tests"
	echo "  -t    TCP/IP command tests"
	echo "  -c    RPC and TI-RPC tests"
	echo "  -d    TS-RPC tests"
	echo "  -a    Application stress tests (HTTP, SSH, DNS)"
	echo "  -e    Interface stress tests"
	echo "  -b    Stress tests with malformed ICMP packets"
	echo "  -i    IPsec ICMP stress tests"
	echo "  -T    IPsec TCP stress tests"
	echo "  -U    IPsec UDP stress tests"
	echo "  -D    IPsec DCCP stress tests"
	echo "  -S    IPsec SCTP stress tests"
	echo "  -R    route stress tests"
	echo "  -M    multicast stress tests"
	echo "  -F    network features tests (TFO, vxlan, etc.)"
	echo "  -f x  where x is a runtest file"
	echo "  -q    quiet mode (this implies not logging start of test"
	echo "        in kernel log)"
	echo "  -Q    don't log start of test in kernel log"
	echo "  -V|v  verbose"
	echo "  -h    print this help"
}

while getopts 6mnrstaebcdiTUDSRMFf:qQVvh OPTION
do
	case $OPTION in
	6) TEST_CASES="$TEST_CASES net.ipv6 net.ipv6_lib";;
	m) TEST_CASES="$TEST_CASES net.multicast";;
	n) TEST_CASES="$TEST_CASES net.nfs";;
	s) TEST_CASES="$TEST_CASES net.sctp";;
	t) TEST_CASES="$TEST_CASES net.tcp_cmds";;
	c) TEST_CASES="$TEST_CASES net.rpc_tests";;
	d) TEST_CASES="$TEST_CASES net.tirpc_tests";;
	a) TEST_CASES="$TEST_CASES net_stress.appl";;
	e) TEST_CASES="$TEST_CASES net_stress.interface";;
	b) TEST_CASES="$TEST_CASES net_stress.broken_ip";;
	i) TEST_CASES="$TEST_CASES net_stress.ipsec_icmp";;
	T) TEST_CASES="$TEST_CASES net_stress.ipsec_tcp";;
	U) TEST_CASES="$TEST_CASES net_stress.ipsec_udp";;
	D) TEST_CASES="$TEST_CASES net_stress.ipsec_dccp";;
	S) TEST_CASES="$TEST_CASES net_stress.ipsec_sctp";;
	R) TEST_CASES="$TEST_CASES net_stress.route";;
	M) TEST_CASES="$TEST_CASES net_stress.multicast";;
	F) TEST_CASES="$TEST_CASES net.features";;
	f) TEST_CASES=${OPTARG};;
	q) QUIET_MODE="-q";;
	Q) NO_KMSG="-Q";;
	V|v) VERBOSE="yes";;
	h) usage; exit 0;;
	*) echo "Error: invalid option..."; usage; exit 1;;
	esac
done

if [ "$OPTIND" -eq 1 ]; then
	echo "Error: option is required"
	usage
	exit 1
fi
shift $(($OPTIND - 1))

TST_NO_DEFAULT_RUN=1
. tst_net.sh

# Reset variables.
# Don't break the tests which are using 'testcases/lib/cmdlib.sh'
unset TST_ID TST_LIB_LOADED TST_NO_DEFAULT_RUN

rm -f $CMDFILE

for t in $TEST_CASES; do
	cat  ${LTPROOT}/runtest/$t >> $CMDFILE
done

cd $TMPDIR

cmd="${LTPROOT}/bin/ltp-pan $QUIET_MODE $NO_KMSG -e -l /tmp/netpan.log -S -a ltpnet -n ltpnet -f $CMDFILE"

if [ ${VERBOSE} = "yes" ]; then
	echo "Network parameters:"
	echo " - ${LHOST_IFACES} local interface (MAC address: ${LHOST_HWADDRS})"
	echo " - ${RHOST_IFACES} remote interface (MAC address: ${RHOST_HWADDRS})"

	cat $CMDFILE
	${LTPROOT}/ver_linux
	echo ""
	echo $cmd
fi

$cmd

if [ $? -eq "0" ]; then
	echo ltp-pan reported PASS
else
	echo ltp-pan reported FAIL
fi

rm -rf $TMPDIR
