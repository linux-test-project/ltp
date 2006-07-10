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

# ---***** THESE MUST BE SET FOR CORRECT OPERATION *****---
export RHOST=
export LHOST_HWADDRS= 
export RHOST_HWADDRS=
export HTTP_DOWNLOAD_DIR=
export FTP_DOWNLOAD_DIR=
export FTP_UPLOAD_DIR=
export FTP_UPLOAD_URLDIR=
# ---***************************************************---=

# ---***** OPTIONAL SETTINGS *****---
# DEFAULTS
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
# ---*********************************---


export TMPDIR=/tmp/netst-$$
mkdir $TMPDIR

# The total number of the stress test is quite big.
# If you'd like to reduce the number of the stress test, you can choose
# network_stress.selected, or you can choose network_stress.<category> 
# Of cource, you can use your custumized command file to run the tests
# that you're interested in.
cat ${LTPROOT}/runtest/network_stress.whole > $TMPDIR/network_stress.tests
#cat ${LTPROOT}/runtest/network_stress.selected > $TMPDIR/network_stress.tests
#cat (( cutimized command file location )) > $TMPDIR/network_stress.tests

cd $TMPDIR

export PATH="${PATH}:${LTPROOT}/testcases/bin"
 
${LTPROOT}/ver_linux

${LTPROOT}/pan/pan -e -l /tmp/netstress.log -S -a netstress -n netstress -f ${TMPDIR}/network_stress.tests

if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi

rm -rf ${TMPDIR}
