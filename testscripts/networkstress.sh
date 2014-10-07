#!/bin/sh
# This will run all the network stress tests, with the status logged in
# /tmp/netpan.log
#
# Please read ltp-yyyymmdd/testcases/network/stress/README before running

cd $(dirname $0)
. ./network.sh

# Test Settings
export NS_DURATION=${NS_DURATION:-"3600"}
export NS_TIMES=${NS_TIMES:-"10000"}
export CONNECTION_TOTAL=${CONNECTION_TOTAL:-"4000"}
export IP_TOTAL=${IP_TOTAL:-"10000"}
export IP_TOTAL_FOR_TCPIP=${IP_TOTAL_FOR_TCPIP:-"100"}
export ROUTE_TOTAL=${ROUTE_TOTAL:-"10000"}
export MTU_CHANGE_TIMES=${MTU_CHANGE_TIMES:-"1000"}
export IF_UPDOWN_TIMES=${IF_UPDOWN_TIMES:-"10000"}
export DOWNLOAD_BIGFILESIZE=${DOWNLOAD_BIGFILESIZE:-"2147483647"}
export DOWNLOAD_REGFILESIZE=${DOWNLOAD_REGFILESIZE:-"1048576"}
export UPLOAD_BIGFILESIZE=${UPLOAD_BIGFILESIZE:-"2147483647"}
export UPLOAD_REGFILESIZE=${UPLOAD_REGFILESIZE:-"1024"}
export MCASTNUM_NORMAL=${MCASTNUM_NORMAL:-"20"}
export MCASTNUM_HEAVY=${MCASTNUM_HEAVY:-"40000"}

usage () {
    echo ""
    echo "---------------------------------------------------------"
    echo -e "\033[31m $0 [options] \033[0m "
    echo "---------------------------------------------------------"
    echo " -A|a: Stress test for appl"
    echo " -E|e: Stress test for interface"
    echo " -I|i: Stress test for ICMP protocol"
    echo " -T|t: Stress test for TCP/IP"
    echo " -U|u: Stress test for UDP/IP"
    echo " -R|r: Stress test for routing table"
    echo " -B|b: Stress Broken IP packets"
    echo " -M|m: Multicast stress tests"
    echo " -F|f: Stress test for network features"
    echo " -S|s: Run selected tests"
    echo " -W|w: Run whole network stress tests"
    echo " -D|d: Test duration (default ${NS_DURATION} sec)"
    echo " -V|v: Enable verbose"
    echo " -H|h: This Usage"
    echo ""
    exit 1
}

while getopts AaEeTtIiUuRrMmFfSsWwBbVvD:d: OPTION
do
    case $OPTION in
	A|a) TEST_CASE="network_stress.appl";;
	E|e) TEST_CASE="network_stress.interface";;
	B|b) TEST_CASE="network_stress.broken_ip";;
	I|i) TEST_CASE="network_stress.icmp";;
	T|t) TEST_CASE="network_stress.tcp";;
	U|u) TEST_CASE="network_stress.udp";;
	R|r) TEST_CASE="network_stress.route";;
	M|m) TEST_CASE="network_stress.multicast";;
	F|f) TEST_CASE="network_stress.features";;
	S|s) TEST_CASE="network_stress.selected";;
	W|w) TEST_CASE="network_stress.whole";;
	V|v) VERBOSE="yes";;
	D|d) NS_DURATION=${OPTARG};;
	H|h) usage;;
	*) echo "Error: invalid option..."; usage; exit 1 ;;
    esac
done

if [ -z ${TEST_CASE} ]; then
	usage
fi

if [ -z ${RHOST} ]; then
	## Just a silly check
	echo "Error: pay attention to configure"
	echo "  network paramaters before running tests."
	exit 1
fi

cat ${LTPROOT}/runtest/${TEST_CASE} > $TMPDIR/network_stress.tests

cd $TMPDIR

if [ ${VERBOSE} = "yes" ]; then
	echo "Network parameters:"
	echo " - ${LHOST_IFACES} local interface (MAC address: ${LHOST_HWADDRS})"
	echo " - ${RHOST_IFACES} remote interface (MAC address: ${RHOST_HWADDRS})"

	cat $TMPDIR/network_stress.tests
	${LTPROOT}/ver_linux
	echo ""
	echo ${LTPROOT}/bin/ltp-pan -e -l /tmp/netstress.log -S -a netstress -n netstress -f ${TMPDIR}/network_stress.tests
fi

${LTPROOT}/bin/ltp-pan -e -l /tmp/netstress.log -S -a netstress -n netstress -f ${TMPDIR}/network_stress.tests

if [ $? -eq "0" ]; then
  echo ltp-pan reported PASS
else
  echo ltp-pan reported FAIL
fi

rm -rf ${TMPDIR}
