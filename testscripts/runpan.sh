#!/bin/sh
# This will only run the quickhit tests.  
cd `dirname $0`
LTPROOT=${PWD}

mkdir /tmp/runpan-$$
cd /tmp/runpan-$$

export PATH="${PATH}:${LTPROOT}/doio:${LTPROOT}/testcases/bin"
 
${LTPROOT}/pan/pan -e $@ -a ltp -n ltp -f ${LTPROOT}/runtest/quickhit

if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi
