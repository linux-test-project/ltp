#!/bin/sh
# This will run all the network tests, with the status logged in /tmp/netpan.log  
cd `dirname $0`
export LTPROOT=${PWD}
echo $LTPROOT | grep testscripts > /dev/null 2>&1
if [ $? -eq 0 ]; then
 cd ..
 export LTPROOT=${PWD}
fi


# ---***** THESE MUST BE SET FOR CORRECT OPERATION *****---
export RHOST=
export PASSWD=
# ---***************************************************---

# ---***** NFS OPTIONAL SETTINGS *****---
# DEFAULTS
# export VERSION=2 
# export SOCKET_TYPE=udp
# ---*********************************---


export TMPDIR=/tmp/netpan-$$
mkdir $TMPDIR

cat  ${LTPROOT}/runtest/tcp_cmds > $TMPDIR/network.tests
cat  ${LTPROOT}/runtest/multicast >> $TMPDIR/network.tests
cat  ${LTPROOT}/runtest/rpc >> $TMPDIR/network.tests
cat  ${LTPROOT}/runtest/nfs >> $TMPDIR/network.tests

cd $TMPDIR

export PATH="${PATH}:${LTPROOT}/testcases/bin"
 
${LTPROOT}/ver_linux

${LTPROOT}/pan/pan -e -l /tmp/netpan.log -S -a ltpnet -n ltpnet -f ${TMPDIR}/network.tests

if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi

rm -rf ${TMPDIR}
