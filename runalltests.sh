#!/bin/sh

cd `dirname $0`
export LTPROOT=${PWD}
export TMP="/tmp/runalltests-$$"

mkdir ${TMP}
cd ${TMP}

export PATH="${PATH}:${LTPROOT}/testcases/bin"

cat ${LTPROOT}/runtest/syscalls ${LTPROOT}/runtest/fs ${LTPROOT}/runtest/mm ${LTPROOT}/runtest/commands ${LTPROOT}/runtest/ipc ${LTPROOT}/runtest/sched ${LTPROOT}/runtest/float > ${TMP}/alltests

# Uncomment below and define if execution of the fsx-linux tests is desired.
# Example:
# export SCRATCHDEV=/dev/hda

# Uncomment the next line if SCRATCHDEV defined
#cat ${LTPROOT}/runtest/fsx >> ${TMP}/alltests

${LTPROOT}/ver_linux

${LTPROOT}/pan/pan -e -S -a $$ -n $$ -f ${TMP}/alltests

if [ $? -eq "0" ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi

rm -rf ${TMP}
