#!/bin/sh

cd `dirname $0`
LTPROOT=${PWD}
TMP="/tmp/runalltests-$$"

mkdir ${TMP}
cd ${TMP}

export PATH="${PATH}:${LTPROOT}/doio:${LTPROOT}/tests"

cat ${LTPROOT}/runtest/quickhit ${LTPROOT}/runtest/fs ${LTPROOT}/runtest/mm > ${TMP}/alltests

${LTPROOT}/pan/pan -e -S -a $$ -n $$ -f ${TMP}/alltests

if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi

rm -rf ${TMP}
