#!/bin/sh
# This will run all the network tests, with the status logged in /tmp/netpan.log  
cd `dirname $0`
export LTPROOT=${PWD}


# ---***** THESE MUST BE SET FOR CORRECT OPERATION *****---
export RHOST=
export PASSWD=
# ---***************************************************---


export TMPDIR=/tmp

cat  ${LTPROOT}/runtest/tcp_cmds > $TMPDIR/network.tests
cat  ${LTPROOT}/runtest/multicast >> $TMPDIR/network.tests
cat  ${LTPROOT}/runtest/rpc >> $TMPDIR/network.tests
cat  ${LTPROOT}/runtest/nfs >> $TMPDIR/network.tests

mkdir /tmp/netpan-$$
cd /tmp/netpan-$$

export PATH="${PATH}:${LTPROOT}/testcases/bin"
 
${LTPROOT}/ver_linux

${LTPROOT}/pan/pan -e -l /tmp/netpan.log -S -a ltpnet -n ltpnet -f ${TMPDIR}/network.tests

if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi

rm -f ${TMPDIR}/network.tests
