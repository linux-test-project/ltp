#!/bin/sh
# This will only run the quickhit tests.  
cd `dirname $0`
LTPROOT=${PWD}

mkdir /tmp/runalltests-$$
cd /tmp/runalltests-$$

export PATH="${PATH}:${LTPROOT}/doio:${LTPROOT}/tests"
 
${LTPROOT}/pan/pan -e -S -a $$ -n $$ -f ${LTPROOT}/runtest/quickhit

if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi

rm -rf /tmp/runalltests-$$
