#!/bin/sh
# This will run all the network tests, with the status logged in /tmp/netpan.log

# ---***** THESE MUST BE SET FOR CORRECT OPERATION *****---
export RHOST=
export PASSWD=
# ---***************************************************---

cd `dirname $0`
export LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]; then
 cd ..
 export LTPROOT=${PWD}
fi

export TMPDIR=/tmp/netpan-$$
mkdir -p $TMPDIR
CMDFILE=${TMPDIR}/network.tests
VERBOSE="no"

# For bitwise operation to determine which testsets run
CMD_IPV6=1		# 0x0001
CMD_IPV6_LIB=2		# 0x0002
CMD_MULTICAST=4		# 0x0004
CMD_NFS=8		# 0x0008
CMD_RPC=16		# 0x0010
CMD_SCTP=32		# 0x0020
CMD_TCPCMDS=64		# 0x0040
CMD_TCPCMDS_ADD=128	# 0x0080
CMD_WHOLE=65535		# 0xFFFF
CMD_DEFAULT=$(($CMD_MULTICAST|$CMD_NFS|$CMD_RPC|$CMD_TCPCMDS))

# ---***** NFS OPTIONAL SETTINGS *****---
# DEFAULTS
# export VERSION=2 
# export SOCKET_TYPE=udp
# ---*********************************---

usage() {
    echo ""
    echo "---------------------------------------------------------"
    echo -e "\033[31m $0 [options] \033[0m "
    echo "---------------------------------------------------------"
    echo " -W|w: whole network tests"
    echo " -D|d: default network tests"
    echo " -6:   IPv6 tests"
    echo " -L|l: IPv6 library tests"
    echo " -M|m: multicast tests"
    echo " -N|n: nfs tests"
    echo " -R|r: rpc tests"
    echo " -S|s: sctp tests"
    echo " -T|t: TCP/IP command tests"
    echo " -V|v: Enable verbose"
    echo " -H|h: This Usage"
    echo "*) 'default' runs multicast, rpc, nfs and some of TCP/IP command"
    echo "   It is interpreted as 'default' if no test sets are specified"
    echo ""
}

# Parse options
CMD=0
while getopts WwDd6LlMmNnRrSsTtVvHh OPTION
do
    case $OPTION in
	W|w) CMD=$CMD_WHOLE;;
	D|d) CMD=$(($CMD|$CMD_DEFAULT));;
	6)   CMD=$(($CMD|$CMD_IPV6));;
	L|l) CMD=$(($CMD|$CMD_IPV6_LIB));;
	M|m) CMD=$(($CMD|$CMD_MULTICAST));;
	N|n) CMD=$(($CMD|$CMD_NFS));;
	R|r) CMD=$(($CMD|$CMD_RPC));;
	S|s) CMD=$(($CMD|$CMD_SCTP));;
	T|t) CMD=$(($CMD|$CMD_TCPCMDS|$CMD_TCPCMDS_ADD));;
	V|v) VERBOSE="yes";;
	H|h) usage; exit 0 ;;
	*) echo "Error: invalid option..."; usage; exit 1 ;;
    esac
done
if [ $CMD -eq 0 ]; then
    CMD=$CMD_DEFAULT
fi

# Determine which test set will run
rm -f $CMDFILE
if [ $(($CMD&$CMD_IPV6)) -ne 0 ]; then
    cat  ${LTPROOT}/runtest/ipv6 >> $CMDFILE
fi
if [ $(($CMD&$CMD_IPV6_LIB)) -ne 0 ]; then
    cat  ${LTPROOT}/runtest/ipv6_lib >> $CMDFILE
fi
if [ $(($CMD&$CMD_MULTICAST)) -ne 0 ]; then
    cat  ${LTPROOT}/runtest/multicast >> $CMDFILE
fi
if [ $(($CMD&$CMD_NFS)) -ne 0 ]; then
    cat  ${LTPROOT}/runtest/nfs >> $CMDFILE
fi
if [ $(($CMD&$CMD_RPC)) -ne 0 ]; then
    cat  ${LTPROOT}/runtest/rpc >> $CMDFILE
fi
if [ $(($CMD&$CMD_SCTP)) -ne 0 ]; then
    cat  ${LTPROOT}/runtest/sctp >> $CMDFILE
fi
if [ $(($CMD&$CMD_TCPCMDS)) -ne 0 ]; then
    cat  ${LTPROOT}/runtest/tcp_cmds >> $CMDFILE
fi
if [ $(($CMD&$CMD_TCPCMDS_ADD)) -ne 0 ]; then
    cat  ${LTPROOT}/runtest/tcp_cmds_addition >> $CMDFILE
fi

cd $TMPDIR

export PATH="${PATH}:${LTPROOT}/testcases/bin"

if [ ${VERBOSE} = "yes" ]; then
    echo "Network parameters:"
    echo " - Remote Host: ${RHOST}"

    cat $CMDFILE
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

rm -rf ${TMPDIR}
