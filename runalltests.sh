#!/bin/sh

cd `dirname $0`
export LTPROOT=${PWD}
export TMP="/tmp/runalltests-$$"

mkdir ${TMP}
cd ${TMP}

export PATH="${PATH}:${LTPROOT}/testcases/bin"

cat ${LTPROOT}/runtest/syscalls ${LTPROOT}/runtest/fs ${LTPROOT}/runtest/dio ${LTPROOT}/runtest/mm ${LTPROOT}/runtest/commands ${LTPROOT}/runtest/ipc ${LTPROOT}/runtest/sched ${LTPROOT}/runtest/float > ${TMP}/alltests

# The fsx-linux tests use the SCRATCHDEV environment variable as a location
# that can be reformatted and run on.  Set SCRATCHDEV if you want to run 
# these tests.  As a safeguard, this is disabled.
unset SCRATCHDEV
if [ -n "$SCRATCHDEV" ]; then
  cat ${LTPROOT}/runtest/fsx >> ${TMP}/alltests
fi

${LTPROOT}/ver_linux

${LTPROOT}/pan/pan -e -S -a $$ -n $$ -f ${TMP}/alltests

if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi

rm -rf ${TMP}
