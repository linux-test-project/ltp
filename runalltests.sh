#!/bin/sh

cd `dirname $0`
export LTPROOT=${PWD}
TMP="/tmp/runalltests-$$"

mkdir ${TMP}
cd ${TMP}

export PATH="${PATH}:${LTPROOT}/doio:${LTPROOT}/tests"

cat ${LTPROOT}/runtest/syscalls ${LTPROOT}/runtest/fs ${LTPROOT}/runtest/mm ${LTPROOT}/runtest/commands ${LTPROOT}/runtest/ipc ${LTPROOT}/runtest/sched > ${TMP}/alltests

${LTPROOT}/pan/pan -e -S -a $$ -n $$ -f ${TMP}/alltests

if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi

rm -rf ${TMP}
