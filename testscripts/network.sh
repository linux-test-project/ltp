#!/bin/sh

TST_TOTAL=1
TCID="network_settings"

cd $(dirname $0)
export LTPROOT=${LTPROOT:-"$PWD"}
echo $LTPROOT | grep -q testscripts
if [ $? -eq 0 ]; then
	cd ..
	export LTPROOT=${PWD}
fi

. test_net.sh

export TMPDIR=/tmp/netpan-$$
mkdir -p $TMPDIR
CMDFILE=${TMPDIR}/network.tests
VERBOSE="no"

export PATH="${PATH}:${LTPROOT}/testcases/bin"

# Reset variables.
# Don't break the tests which are using 'testcases/lib/cmdlib.sh'
export TCID=
export TST_LIB_LOADED=

TEST_CASES=

while getopts 6mnrstaebiTURMFf:Vv OPTION
do
	case $OPTION in
	6) TEST_CASES="$TEST_CASES ipv6 ipv6_lib";;
	m) TEST_CASES="$TEST_CASES multicast" ;;
	n) TEST_CASES="$TEST_CASES nfs" ;;
	r) TEST_CASES="$TEST_CASES rpc" ;;
	s) TEST_CASES="$TEST_CASES sctp" ;;
	t) TEST_CASES="$TEST_CASES tcp_cmds" ;;
	a) TEST_CASES="$TEST_CASES network_stress.appl";;
	e) TEST_CASES="$TEST_CASES network_stress.interface";;
	b) TEST_CASES="$TEST_CASES network_stress.broken_ip";;
	i) TEST_CASES="$TEST_CASES network_stress.icmp";;
	T) TEST_CASES="$TEST_CASES network_stress.tcp";;
	U) TEST_CASES="$TEST_CASES network_stress.udp";;
	R) TEST_CASES="$TEST_CASES network_stress.route";;
	M) TEST_CASES="$TEST_CASES network_stress.multicast";;
	F) TEST_CASES="$TEST_CASES network_stress.features";;
	f) TEST_CASES=${OPTARG} ;;
	V|v) VERBOSE="yes";;
	*) echo "Error: invalid option..."; exit 1 ;;
	esac
done

rm -f $CMDFILE

for t in $TEST_CASES; do
	cat  ${LTPROOT}/runtest/$t >> $CMDFILE
done

cd $TMPDIR

if [ ${VERBOSE} = "yes" ]; then
	echo "Network parameters:"
	echo " - ${LHOST_IFACES} local interface (MAC address: ${LHOST_HWADDRS})"
	echo " - ${RHOST_IFACES} remote interface (MAC address: ${RHOST_HWADDRS})"

	cat $TMPDIR/network_stress.tests
	${LTPROOT}/ver_linux
	echo ""
	echo ${LTPROOT}/bin/ltp-pan -e -l /tmp/netpan.log -S -a ltpnet -n ltpnet -f $CMDFILE
fi

${LTPROOT}/bin/ltp-pan -e -l /tmp/netpan.log -S -a ltpnet -n ltpnet -f $CMDFILE

if [ $? -eq "0" ]; then
	echo ltp-pan reported PASS
else
	echo ltp-pan reported FAIL
fi
