#!/bin/sh

cd `dirname $0`
LTPROOT=${PWD}

mkdir /tmp/runalltests-$$
cd /tmp/runalltests-$$

export PATH="${PATH}:${LTPROOT}/doio:${LTPROOT}/tests"

python ${LTPROOT}/runtest/runtests.py ${LTPROOT}/runtest/quickhit ${LTPROOT}/runtest/fs

rm -rf /tmp/runalltests-$$

if [ $? -eq "0" ]; then
  echo runtests reported PASS
else
  echo runtests reported FAIL
fi
