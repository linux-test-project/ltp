#!/bin/sh
# This will run all the network stress tests, with the status logged in
# /tmp/netpan.log  
#
# Please read ltp-yyyymmdd/testcases/network/stress/README before running

cd `dirname $0`
export LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]; then
 cd ..
 export LTPROOT=${PWD}
fi

export TMPDIR=/tmp/netst-$$
mkdir $TMPDIR
VERBOSE="no"
INTERFACE="eth0"

#===========================================================================
# Network parameters
export RHOST=
export RHOST_HWADDRS=
export HTTP_DOWNLOAD_DIR=
export FTP_DOWNLOAD_DIR=
export FTP_UPLOAD_DIR=
export FTP_UPLOAD_URLDIR=

# Set firt three octets of the network address, by default 10.0.0
export IPV4_NETWORK=
# Set local host last octet, by default 2
export LHOST_IPV4_HOST=
# Set remote host last octet, by default 1
export RHOST_IPV4_HOST=
# Set the reverse of IPV4_NETWORK, by default 0.0.10
export IPV4_NETWORK_REVERSE=

#===========================================================================
# Default Test Settings
# export LTP_RSH=rsh
# export NS_DURATION=3600	# 1 hour
# export NS_TIMES=10000
# export CONNECTION_TOTAL=4000
# export IP_TOTAL=10000
# export IP_TOTAL_FOR_TCPIP=100 
# export ROUTE_TOTAL=10000
# export MTU_CHANGE_TIMES=1000
# export IF_UPDOWN_TIMES=10000
# export DOWNLOAD_BIGFILESIZE=2147483647	# 2G byte - 1byte
# export DOWNLOAD_REGFILESIZE=1048576		# 1M byte
# export UPLOAD_BIGFILESIZE=2147483647		# 2G byte - 1byte
# export UPLOAD_REGFILESIZE=1024		# 1K byte
# export MCASTNUM_NORMAL=20
# export MCASTNUM_HEAVY=40000
#===========================================================================

usage () {
    echo ""
    echo "---------------------------------------------------------"
    echo -e "\033[31m $0 [options] \033[0m "
    echo "---------------------------------------------------------"
    echo " -E|e: Stress test for interface"
    echo " -I|i: Stress test for ICMP protocol"
    echo " -T|t: Stress test for TCP/IP"
    echo " -U|u: Stress test for UDP/IP"
    echo " -R|r: Stress test for routing table"
    echo " -B|b: Stress Broken IP packets"
    echo " -M|m: Multicast stress tests"
    echo " -S|s: Run selected tests"
    echo " -W|w: Run whole network stress tests"
    echo " -D|d: Test duration (default ${NS_DURATION} sec)"
    echo " -N|n: Select the network interface (default: $INTERFACE)"
    echo " -V|v: Enable verbose"
    echo " -H|h: This Usage"
    echo ""
    exit 1
}

while getopts EeTtIiUuRrMmSsWwBbVvN:n:D:d: OPTION
do
    case $OPTION in
	E|e) TEST_CASE="network_stress.interface";;
	B|b) TEST_CASE="network_stress.broken_ip";;
	I|i) TEST_CASE="network_stress.icmp";;
	T|t) TEST_CASE="network_stress.tcp";;
	U|u) TEST_CASE="network_stress.udp";;
	R|r) TEST_CASE="network_stress.route";;
	M|m) TEST_CASE="network_stress.multicast";;
	S|s) TEST_CASE="network_stress.selected";;
	W|w) TEST_CASE="network_stress.whole";;
	V|v) VERBOSE="yes";;
	N|n) INTERFACE=${OPTARG};;
	D|d) NS_DURATION=${OPTARG};;
	H|h) usage;;
	*) echo "Error: invalid option..."; usage; exit 1 ;;
    esac
done

if [ -z ${TEST_CASE} ]; then
	usage
fi

export LHOST_HWADDRS=`ifconfig | grep ${INTERFACE} | grep HWaddr |awk '{print $5}'`

if [ -z ${RHOST} ]; then
	## Just a silly check
	echo "Error: pay attention to configure"
	echo "  network paramaters before running tests."
	exit 1
fi

cat ${LTPROOT}/runtest/${TEST_CASE} > $TMPDIR/network_stress.tests

cd $TMPDIR

export PATH="${PATH}:${LTPROOT}/testcases/bin"
 
if [ ${VERBOSE} = "yes" ]; then
	echo "Network parameters:"
	echo " - ${INTERFACE} local interface (MAC address: ${LHOST_HWADDRS})"
	echo " - Remote IP address: ${RHOST}"
	echo " - Remote MAC address: ${RHOST_HWADDRS}"

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
